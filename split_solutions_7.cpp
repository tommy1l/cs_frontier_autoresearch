#include <bits/stdc++.h>
using namespace std;

struct Trip { int m; long long lo, hi; };

struct Graph {
    int n = 0;
    vector<vector<pair<int,int>>> adj;
    int START, END;
    vector<vector<Trip>> nodeProfile;
    vector<bool> deleted;
    
    int newNode() {
        n++;
        adj.push_back({});
        nodeProfile.push_back({});
        return n;
    }
};

string serialize(vector<Trip>& P) {
    sort(P.begin(), P.end(), [](const Trip& a, const Trip& b){
        if (a.m != b.m) return a.m < b.m;
        if (a.lo != b.lo) return a.lo < b.lo;
        return a.hi < b.hi;
    });
    string s;
    for (auto& t : P) {
        s += to_string(t.m); s += '-';
        s += to_string(t.lo); s += '-';
        s += to_string(t.hi); s += '-';
    }
    return s;
}

int buildNode(Graph& G, vector<Trip> profile, map<string,int>& memo) {
    if (profile.empty()) return -1;
    string key = serialize(profile);
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    int node = G.newNode();
    memo[key] = node;
    G.nodeProfile[node-1] = profile;
    
    for (int b = 0; b < 2; b++) {
        vector<Trip> child;
        bool stopHere = false;
        for (auto& t : profile) {
            long long half = 1LL << (t.m - 1);
            long long clo, chi;
            if (b == 0) {
                clo = max(0LL, t.lo);
                chi = min(half - 1, t.hi);
            } else {
                clo = max(0LL, t.lo - half);
                chi = min(half - 1, t.hi - half);
            }
            if (clo > chi) continue;
            if (t.m == 1) {
                if (clo == 0 && chi == 0) stopHere = true;
            } else {
                child.push_back({t.m - 1, clo, chi});
            }
        }
        if (stopHere) G.adj[node-1].push_back({G.END, b});
        if (!child.empty()) {
            int cid = buildNode(G, child, memo);
            if (cid != -1) G.adj[node-1].push_back({cid, b});
        }
    }
    return node;
}

vector<int> topoOrder(Graph& G) {
    vector<int> indeg(G.n+1, 0);
    for (int u = 1; u <= G.n; u++) {
        if (G.deleted[u]) continue;
        for (auto& e : G.adj[u-1]) {
            if (!G.deleted[e.first]) indeg[e.first]++;
        }
    }
    queue<int> q;
    for (int u = 1; u <= G.n; u++) {
        if (!G.deleted[u] && indeg[u] == 0) q.push(u);
    }
    vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (auto& e : G.adj[u-1]) {
            if (G.deleted[e.first]) continue;
            if (--indeg[e.first] == 0) q.push(e.first);
        }
    }
    return order;
}

vector<vector<uint64_t>> computeReach(Graph& G) {
    int W = (G.n + 1 + 63) / 64;
    vector<vector<uint64_t>> reach(G.n+1, vector<uint64_t>(W, 0));
    vector<int> order = topoOrder(G);
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        for (auto& e : G.adj[u-1]) {
            int c = e.first;
            if (G.deleted[c]) continue;
            reach[u][c >> 6] |= (1ULL << (c & 63));
            for (int w = 0; w < W; w++) reach[u][w] |= reach[c][w];
        }
    }
    return reach;
}

long long totalPathCount(Graph& G, long long cap, int maxLen) {
    vector<vector<long long>> cnt(G.n+1, vector<long long>(maxLen+2, 0));
    cnt[G.END][0] = 1;
    vector<int> order = topoOrder(G);
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        if (u == G.END) continue;
        for (int k = 1; k <= maxLen+1; k++) {
            long long s = 0;
            for (auto& e : G.adj[u-1]) {
                int c = e.first;
                if (G.deleted[c]) continue;
                s += cnt[c][k-1];
                if (s > cap) { s = cap+1; break; }
            }
            cnt[u][k] = min(s, cap+1);
        }
    }
    long long total = 0;
    for (int k = 0; k <= maxLen+1; k++) {
        total += cnt[G.START][k];
        if (total > cap) return cap+1;
    }
    return total;
}

bool valueIntervalDisjoint(Graph& G, int u, int v) {
    map<int, vector<pair<long long,long long>>> byM;
    for (auto& t : G.nodeProfile[u-1]) byM[t.m].push_back({t.lo, t.hi});
    for (auto& t : G.nodeProfile[v-1]) byM[t.m].push_back({t.lo, t.hi});
    for (auto& kv : byM) {
        if (kv.second.size() < 2) continue;
        long long maxLo = LLONG_MIN, minHi = LLONG_MAX;
        for (auto& p : kv.second) {
            maxLo = max(maxLo, p.first);
            minHi = min(minHi, p.second);
        }
        if (maxLo <= minHi) return false;
    }
    return true;
}

void mergePass(Graph& G, long long target, long long cap, int maxLen) {
    bool progress = true;
    while (progress) {
        progress = false;
        auto reach = computeReach(G);
        for (int u = 1; u <= G.n && !progress; u++) {
            if (G.deleted[u] || u == G.START || u == G.END) continue;
            for (int v = u+1; v <= G.n && !progress; v++) {
                if (G.deleted[v] || v == G.START || v == G.END) continue;
                if (G.nodeProfile[u-1].empty() || G.nodeProfile[v-1].empty()) continue;
                if (!valueIntervalDisjoint(G, u, v)) continue;
                if (reach[u][v>>6] & (1ULL << (v&63))) continue;
                if (reach[v][u>>6] & (1ULL << (u&63))) continue;
                
                set<pair<int,int>> merged;
                bool bad = false;
                for (auto& e : G.adj[u-1]) {
                    if (G.deleted[e.first]) { bad = true; break; }
                    if (e.first == u) { bad = true; break; }
                    merged.insert(e);
                }
                if (bad) continue;
                for (auto& e : G.adj[v-1]) {
                    if (G.deleted[e.first]) { bad = true; break; }
                    if (e.first == v) { bad = true; break; }
                    merged.insert(e);
                }
                if (bad) continue;
                if (merged.size() > 200) continue;
                
                vector<pair<int,int>> snapU = G.adj[u-1];
                vector<pair<int,int>> snapV = G.adj[v-1];
                vector<bool> snapDel = G.deleted;
                vector<pair<int, vector<pair<int,int>>>> snapPred;
                for (int p = 1; p <= G.n; p++) {
                    if (G.deleted[p]) continue;
                    bool has = false;
                    for (auto& e : G.adj[p-1]) if (e.first == v) { has = true; break; }
                    if (has) snapPred.push_back({p, G.adj[p-1]});
                }
                
                G.adj[u-1].assign(merged.begin(), merged.end());
                int origProfSize = (int)G.nodeProfile[u-1].size();
                for (auto& t : G.nodeProfile[v-1]) G.nodeProfile[u-1].push_back(t);
                
                for (auto& sp : snapPred) {
                    int p = sp.first;
                    set<pair<int,int>> ns;
                    for (auto& e : G.adj[p-1]) {
                        int tgt = (e.first == v) ? u : e.first;
                        if (tgt == p) continue;
                        ns.insert({tgt, e.second});
                    }
                    G.adj[p-1].assign(ns.begin(), ns.end());
                }
                
                G.deleted[v] = true;
                G.adj[v-1].clear();
                
                long long pc = totalPathCount(G, cap, maxLen);
                if (pc == target) {
                    progress = true;
                } else {
                    G.adj[u-1] = snapU;
                    G.adj[v-1] = snapV;
                    G.deleted = snapDel;
                    for (auto& sp : snapPred) {
                        G.adj[sp.first - 1] = sp.second;
                    }
                    G.nodeProfile[u-1].resize(origProfSize);
                }
            }
        }
    }
}

struct DSU {
    vector<int> p;
    bool tainted = false;
    DSU(int n) : p(n) { iota(p.begin(), p.end(), 0); }
    int find(int x) { while (p[x]!=x){ p[x]=p[p[x]]; x=p[x]; } return x; }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (a < b) p[b] = a; else p[a] = b;
        return true;
    }
};

void batchMergePass(Graph& G, long long target, long long cap, int maxLen) {
    bool progress = true;
    while (progress) {
        progress = false;
        auto reach = computeReach(G);
        int N = G.n;
        
        for (int u = 1; u <= N && !progress; u++) {
            if (G.deleted[u] || u == G.START || u == G.END) continue;
            for (int v = u+1; v <= N && !progress; v++) {
                if (G.deleted[v] || v == G.START || v == G.END) continue;
                if (G.nodeProfile[u-1].empty() || G.nodeProfile[v-1].empty()) continue;
                if (!valueIntervalDisjoint(G, u, v)) continue;
                if (reach[u][v>>6] & (1ULL << (v&63))) continue;
                if (reach[v][u>>6] & (1ULL << (u&63))) continue;
                
                DSU dsu(N+1);
                // members per class
                vector<vector<int>> members(N+1);
                for (int i = 1; i <= N; i++) {
                    if (!G.deleted[i]) members[i].push_back(i);
                }
                
                auto doUnion = [&](int a, int b) -> bool {
                    int ra = dsu.find(a), rb = dsu.find(b);
                    if (ra == rb) return false;
                    int newRoot = min(ra, rb);
                    int oldRoot = max(ra, rb);
                    dsu.p[oldRoot] = newRoot;
                    for (int x : members[oldRoot]) members[newRoot].push_back(x);
                    members[oldRoot].clear();
                    if (newRoot == G.START || newRoot == G.END || oldRoot == G.START || oldRoot == G.END) {
                        // check if class has multiple
                        if (members[newRoot].size() > 1) {
                            for (int x : members[newRoot]) {
                                if (x == G.START || x == G.END) { dsu.tainted = true; break; }
                            }
                        }
                    }
                    return true;
                };
                
                doUnion(u, v);
                if (dsu.tainted) continue;
                
                queue<pair<int,int>> Q;
                Q.push({dsu.find(u), dsu.find(u)});
                // We actually need to inspect class against itself as pairs of its members
                // Reformulate: inspect a single class - propagate constraints among its members
                
                bool abandon = false;
                int qcount = 0;
                while (!Q.empty() && !abandon) {
                    auto pr = Q.front(); Q.pop();
                    qcount++;
                    if (qcount > 4096) { abandon = true; break; }
                    int A = dsu.find(pr.first), B = dsu.find(pr.second);
                    // gather members
                    vector<int> mA = members[A];
                    vector<int> mB;
                    if (A == B) mB = mA; else mB = members[B];
                    
                    for (int a : mA) {
                        if (G.deleted[a]) continue;
                        for (int b : mB) {
                            if (G.deleted[b]) continue;
                            if (a == b) continue;
                            for (int w = 0; w < 2; w++) {
                                int ca = -1, cb = -1;
                                for (auto& e : G.adj[a-1]) if (e.second == w) { ca = e.first; break; }
                                for (auto& e : G.adj[b-1]) if (e.second == w) { cb = e.first; break; }
                                if (ca == -1 || cb == -1) continue;
                                if (G.deleted[ca] || G.deleted[cb]) { abandon = true; break; }
                                int rca = dsu.find(ca), rcb = dsu.find(cb);
                                if (rca != rcb) {
                                    doUnion(ca, cb);
                                    if (dsu.tainted) { abandon = true; break; }
                                    int nr = dsu.find(ca);
                                    Q.push({nr, nr});
                                    if (Q.size() > 4096) { abandon = true; break; }
                                }
                            }
                            if (abandon) break;
                        }
                        if (abandon) break;
                    }
                }
                if (abandon || dsu.tainted) continue;
                
                // Collect classes
                vector<vector<int>> classes;
                for (int i = 1; i <= N; i++) {
                    if (members[i].size() >= 2) classes.push_back(members[i]);
                }
                if (classes.empty()) continue;
                
                // Verify no class contains START/END
                bool valid = true;
                for (auto& cls : classes) {
                    for (int x : cls) {
                        if (x == G.START || x == G.END) { valid = false; break; }
                        if (G.deleted[x]) { valid = false; break; }
                    }
                    if (!valid) break;
                }
                if (!valid) continue;
                
                // Verify no reach cycle within class
                for (auto& cls : classes) {
                    for (size_t i = 0; i < cls.size() && valid; i++) {
                        for (size_t j = 0; j < cls.size() && valid; j++) {
                            if (i == j) continue;
                            int a = cls[i], b = cls[j];
                            if (reach[a][b>>6] & (1ULL << (b&63))) { valid = false; break; }
                        }
                    }
                    if (!valid) break;
                }
                if (!valid) continue;
                
                // Verify merged outdegree
                for (auto& cls : classes) {
                    set<pair<int,int>> outs;
                    for (int x : cls) {
                        for (auto& e : G.adj[x-1]) outs.insert(e);
                    }
                    if (outs.size() > 200) { valid = false; break; }
                }
                if (!valid) continue;
                
                // Snapshot
                vector<vector<pair<int,int>>> snapAdj = G.adj;
                vector<bool> snapDel = G.deleted;
                vector<vector<Trip>> snapProf = G.nodeProfile;
                
                // representative map
                vector<int> rep(N+1, 0);
                for (int i = 1; i <= N; i++) {
                    if (!G.deleted[i]) rep[i] = i;
                }
                for (auto& cls : classes) {
                    int r = *min_element(cls.begin(), cls.end());
                    for (int x : cls) rep[x] = r;
                }
                
                // perform merges
                for (auto& cls : classes) {
                    int r = *min_element(cls.begin(), cls.end());
                    set<pair<int,int>> outs;
                    for (int x : cls) {
                        for (auto& e : G.adj[x-1]) outs.insert(e);
                    }
                    G.adj[r-1].assign(outs.begin(), outs.end());
                    for (int x : cls) {
                        if (x == r) continue;
                        for (auto& t : G.nodeProfile[x-1]) G.nodeProfile[r-1].push_back(t);
                        G.deleted[x] = true;
                        G.adj[x-1].clear();
                    }
                }
                
                // rewire all edges to use rep
                for (int p = 1; p <= N; p++) {
                    if (G.deleted[p]) continue;
                    set<pair<int,int>> ns;
                    for (auto& e : G.adj[p-1]) {
                        int tgt = rep[e.first];
                        if (tgt == p) continue;
                        if (G.deleted[tgt]) continue;
                        ns.insert({tgt, e.second});
                    }
                    G.adj[p-1].assign(ns.begin(), ns.end());
                }
                
                long long pc = totalPathCount(G, cap, maxLen);
                if (pc == target) {
                    progress = true;
                } else {
                    G.adj = snapAdj;
                    G.deleted = snapDel;
                    G.nodeProfile = snapProf;
                }
            }
        }
    }
}

int countAlive(Graph& G) {
    int c = 0;
    for (int i = 1; i <= G.n; i++) if (!G.deleted[i]) c++;
    return c;
}

void outputGraph(Graph& G) {
    vector<int> remap(G.n+1, 0);
    int newN = 0;
    if (!G.deleted[G.START]) { newN++; remap[G.START] = newN; }
    for (int i = 1; i <= G.n; i++) {
        if (G.deleted[i] || i == G.START) continue;
        newN++;
        remap[i] = newN;
    }
    cout << newN << "\n";
    vector<vector<pair<int,int>>> out(newN);
    for (int i = 1; i <= G.n; i++) {
        if (G.deleted[i]) continue;
        int ni = remap[i];
        for (auto& e : G.adj[i-1]) {
            out[ni-1].push_back({remap[e.first], e.second});
        }
    }
    for (int i = 0; i < newN; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) cout << " " << e.first << " " << e.second;
        cout << "\n";
    }
}

void buildForward(Graph& G, long long L, long long R) {
    auto bitlen = [](long long x) {
        int b = 0;
        while (x) { b++; x >>= 1; }
        return b;
    };
    int ellL = bitlen(L), ellR = bitlen(R);
    G.START = G.newNode();
    G.END = G.newNode();
    
    vector<Trip> P1;
    for (int m = ellL - 1; m <= ellR - 1; m++) {
        if (m < 1) continue;
        long long pw = 1LL << m;
        long long lo = max(0LL, L - pw);
        long long hi = min(pw - 1, R - pw);
        if (lo <= hi) P1.push_back({m, lo, hi});
    }
    
    if (L == 1) {
        G.adj[G.START-1].push_back({G.END, 1});
    }
    map<string,int> memo;
    if (!P1.empty()) {
        int c = buildNode(G, P1, memo);
        if (c != -1) G.adj[G.START-1].push_back({c, 1});
    }
    G.deleted.assign(G.n+1, false);
}

bool buildReverseDFA(Graph& Gorig, Graph& NF) {
    int N = Gorig.n;
    vector<vector<pair<int,int>>> revAdj(N+1);
    for (int u = 1; u <= N; u++) {
        if (Gorig.deleted[u]) continue;
        for (auto& e : Gorig.adj[u-1]) {
            int v = e.first, w = e.second;
            revAdj[v].push_back({u, w});
        }
    }
    
    auto serializeVec = [](const vector<int>& v) {
        string s;
        for (int x : v) { s += to_string(x); s += ','; }
        return s;
    };
    
    map<string,int> subsetId;
    vector<vector<int>> subsets;
    vector<int> initSub = {Gorig.END};
    string initKey = serializeVec(initSub);
    subsetId[initKey] = 0;
    subsets.push_back(initSub);
    
    vector<tuple<int,int,int>> transitions;
    queue<int> bfs;
    bfs.push(0);
    
    while (!bfs.empty()) {
        int sid = bfs.front(); bfs.pop();
        vector<int> S = subsets[sid];
        for (int w = 0; w < 2; w++) {
            set<int> Tset;
            for (int u : S) {
                for (auto& p : revAdj[u]) {
                    if (p.second == w) Tset.insert(p.first);
                }
            }
            if (Tset.empty()) continue;
            vector<int> T(Tset.begin(), Tset.end());
            string key = serializeVec(T);
            auto it = subsetId.find(key);
            int tid;
            if (it == subsetId.end()) {
                tid = (int)subsets.size();
                subsetId[key] = tid;
                subsets.push_back(T);
                if ((int)subsets.size() > 200) return false;
                bfs.push(tid);
            } else {
                tid = it->second;
            }
            transitions.push_back({sid, w, tid});
        }
    }
    
    int numSub = (int)subsets.size();
    vector<bool> isAccept(numSub, false);
    for (int i = 0; i < numSub; i++) {
        if (binary_search(subsets[i].begin(), subsets[i].end(), Gorig.START)) {
            isAccept[i] = true;
        }
    }
    
    NF.n = 0;
    NF.adj.clear();
    NF.nodeProfile.clear();
    int mergedStart = NF.newNode();
    vector<int> sid2nf(numSub, -1);
    for (int i = 0; i < numSub; i++) {
        if (isAccept[i]) {
            sid2nf[i] = mergedStart;
        } else {
            sid2nf[i] = NF.newNode();
        }
    }
    NF.START = mergedStart;
    NF.END = sid2nf[0];
    
    set<tuple<int,int,int>> edgeSet;
    for (auto& tr : transitions) {
        int sid, w, tid;
        tie(sid, w, tid) = tr;
        int from = sid2nf[tid];
        int to = sid2nf[sid];
        edgeSet.insert({from, w, to});
    }
    for (auto& e : edgeSet) {
        int from, w, to;
        tie(from, w, to) = e;
        NF.adj[from-1].push_back({to, w});
    }
    
    NF.deleted.assign(NF.n+1, false);
    return true;
}

void computeNFProfiles(Graph& NF) {
    NF.nodeProfile.assign(NF.n, {});
    vector<int> order = topoOrder(NF);
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        if (u == NF.END) continue;
        map<int, vector<pair<long long,long long>>> byM;
        for (auto& e : NF.adj[u-1]) {
            int c = e.first, w = e.second;
            if (c == NF.END) {
                byM[1].push_back({w, w});
            }
            if (c != NF.END) {
                for (auto& t : NF.nodeProfile[c-1]) {
                    byM[t.m + 1].push_back({2*t.lo + w, 2*t.hi + w});
                }
            }
        }
        vector<Trip> prof;
        for (auto& kv : byM) {
            auto& v = kv.second;
            sort(v.begin(), v.end());
            long long curLo = -1, curHi = -1;
            bool has = false;
            for (auto& p : v) {
                if (!has) { curLo = p.first; curHi = p.second; has = true; }
                else if (p.first <= curHi + 1) { curHi = max(curHi, p.second); }
                else { prof.push_back({kv.first, curLo, curHi}); curLo = p.first; curHi = p.second; }
            }
            if (has) prof.push_back({kv.first, curLo, curHi});
        }
        NF.nodeProfile[u-1] = prof;
    }
}

int main() {
    long long L, R;
    cin >> L >> R;
    
    auto bitlen = [](long long x) {
        int b = 0;
        while (x) { b++; x >>= 1; }
        return b;
    };
    
    if (L == R) {
        string bits;
        long long x = L;
        while (x) { bits += char('0' + (x & 1)); x >>= 1; }
        reverse(bits.begin(), bits.end());
        int nn = (int)bits.size() + 1;
        cout << nn << "\n";
        for (int i = 0; i < nn; i++) {
            if (i == nn - 1) cout << 0 << "\n";
            else cout << 1 << " " << (i+2) << " " << (bits[i] - '0') << "\n";
        }
        return 0;
    }
    
    long long target = R - L + 1;
    long long cap = 1LL << 30;
    int maxLen = bitlen(R);
    
    Graph Gf;
    buildForward(Gf, L, R);
    
    Graph Gorig;
    buildForward(Gorig, L, R);
    
    mergePass(Gf, target, cap, maxLen);
    batchMergePass(Gf, target, cap, maxLen);
    int forwardN = countAlive(Gf);
    
    if (forwardN <= 20) {
        outputGraph(Gf);
        return 0;
    }
    
    Graph NF;
    bool ok = buildReverseDFA(Gorig, NF);
    
    if (!ok) {
        outputGraph(Gf);
        return 0;
    }
    
    computeNFProfiles(NF);
    
    mergePass(NF, target, cap, maxLen);
    batchMergePass(NF, target, cap, maxLen);
    int reverseN = countAlive(NF);
    
    if (reverseN < forwardN) {
        outputGraph(NF);
    } else {
        outputGraph(Gf);
    }
    
    return 0;
}