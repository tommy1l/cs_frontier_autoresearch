#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;

struct Trip { int m; long long lo, hi; };

vector<vector<Trip>> nodeProfile;
vector<bool> deleted;

int newNode() {
    n++;
    adj.push_back({});
    nodeProfile.push_back({});
    deleted.push_back(false);
    return n;
}

int END_NODE;
map<string, int> profMemo;

string serialize(vector<Trip>& P) {
    sort(P.begin(), P.end(), [](const Trip& a, const Trip& b){ return a.m < b.m; });
    string s;
    for (auto& t : P) {
        s += to_string(t.m); s += '-';
        s += to_string(t.lo); s += '-';
        s += to_string(t.hi); s += '-';
    }
    return s;
}

int buildNode(vector<Trip> profile, long long v) {
    if (profile.empty()) return -1;
    string key = serialize(profile);
    auto it = profMemo.find(key);
    if (it != profMemo.end()) return it->second;
    int node = newNode();
    profMemo[key] = node;
    nodeProfile[node-1] = profile;
    
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
        if (stopHere) adj[node-1].push_back({END_NODE, b});
        if (!child.empty()) {
            int cid = buildNode(child, 2*v + b);
            if (cid != -1) adj[node-1].push_back({cid, b});
        }
    }
    return node;
}

// reach[u] : bitset of nodes reachable from u (not including u itself)
vector<vector<uint64_t>> reachBits;
int wordsPerBitset;

void computeReach() {
    wordsPerBitset = (n + 63) / 64;
    reachBits.assign(n, vector<uint64_t>(wordsPerBitset, 0));
    
    // topological order
    vector<int> indeg(n, 0);
    for (int u = 0; u < n; u++) {
        if (deleted[u]) continue;
        for (auto& e : adj[u]) {
            int v = e.first - 1;
            if (deleted[v]) continue;
            indeg[v]++;
        }
    }
    vector<int> order;
    queue<int> q;
    for (int u = 0; u < n; u++) if (!deleted[u] && indeg[u] == 0) q.push(u);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (auto& e : adj[u]) {
            int v = e.first - 1;
            if (deleted[v]) continue;
            if (--indeg[v] == 0) q.push(v);
        }
    }
    
    for (int i = order.size() - 1; i >= 0; i--) {
        int u = order[i];
        for (auto& e : adj[u]) {
            int v = e.first - 1;
            if (deleted[v]) continue;
            // set bit v in reach[u]
            reachBits[u][v >> 6] |= (1ULL << (v & 63));
            // OR reach[v]
            for (int w = 0; w < wordsPerBitset; w++) {
                reachBits[u][w] |= reachBits[v][w];
            }
        }
    }
}

bool reachable(int u, int v) {
    return (reachBits[u][v >> 6] >> (v & 63)) & 1ULL;
}

bool profileDisjoint(vector<Trip>& A, vector<Trip>& B) {
    map<int, pair<long long, long long>> ma, mb;
    for (auto& t : A) {
        if (ma.count(t.m)) {
            ma[t.m].first = min(ma[t.m].first, t.lo);
            ma[t.m].second = max(ma[t.m].second, t.hi);
        } else ma[t.m] = {t.lo, t.hi};
    }
    for (auto& t : B) {
        if (mb.count(t.m)) {
            mb[t.m].first = min(mb[t.m].first, t.lo);
            mb[t.m].second = max(mb[t.m].second, t.hi);
        } else mb[t.m] = {t.lo, t.hi};
    }
    for (auto& p : ma) {
        if (mb.count(p.first)) {
            auto& a = p.second;
            auto& b = mb[p.first];
            if (!(a.second < b.first || b.second < a.first)) return false;
        }
    }
    return true;
}

int START_NODE_GLOBAL;

void mergePass() {
    while (true) {
        computeReach();
        
        struct Cand { int u, v, score; };
        vector<Cand> cands;
        
        vector<int> alive;
        for (int u = 0; u < n; u++) {
            if (deleted[u]) continue;
            if (u + 1 == START_NODE_GLOBAL) continue;
            if (u + 1 == END_NODE) continue;
            if (nodeProfile[u].empty()) continue;
            alive.push_back(u);
        }
        
        for (size_t i = 0; i < alive.size(); i++) {
            for (size_t j = i + 1; j < alive.size(); j++) {
                int u = alive[i], v = alive[j];
                if (reachable(u, v) || reachable(v, u)) continue;
                if (!profileDisjoint(nodeProfile[u], nodeProfile[v])) continue;
                int score = (int)adj[u].size() + (int)adj[v].size();
                cands.push_back({u, v, score});
            }
        }
        
        if (cands.empty()) break;
        
        sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b){ return a.score < b.score; });
        
        bool didMerge = false;
        for (auto& c : cands) {
            int u = c.u, v = c.v;
            if (deleted[u] || deleted[v]) continue;
            
            // Check the merged outdegree
            set<pair<int,int>> uni;
            for (auto& e : adj[u]) uni.insert(e);
            for (auto& e : adj[v]) uni.insert(e);
            if ((int)uni.size() > 200) continue;
            
            // Re-check reach (it could have changed but we computed it for this iter; both u and v alive)
            if (reachable(u, v) || reachable(v, u)) continue;
            
            // Merge v into u
            adj[u].clear();
            for (auto& e : uni) adj[u].push_back(e);
            
            // Append v's profile to u's
            for (auto& t : nodeProfile[v]) nodeProfile[u].push_back(t);
            
            // Rewrite predecessors of v to point to u
            for (int p = 0; p < n; p++) {
                if (deleted[p]) continue;
                if (p == v) continue;
                for (auto& e : adj[p]) {
                    if (e.first - 1 == v) e.first = u + 1;
                }
                // dedupe
                if (p != u) {
                    set<pair<int,int>> dd(adj[p].begin(), adj[p].end());
                    adj[p].assign(dd.begin(), dd.end());
                }
            }
            // also dedupe u after potential predecessor rewrites (no, u's adj was set)
            
            deleted[v] = true;
            adj[v].clear();
            nodeProfile[v].clear();
            
            didMerge = true;
            break;
        }
        
        if (!didMerge) break;
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
    
    int ellL = bitlen(L), ellR = bitlen(R);
    
    int START = newNode();
    END_NODE = newNode();
    START_NODE_GLOBAL = START;
    
    vector<Trip> P1;
    for (int m = ellL - 1; m <= ellR - 1; m++) {
        if (m < 1) continue;
        long long pw = 1LL << m;
        long long lo = max(0LL, L - pw);
        long long hi = min(pw - 1, R - pw);
        if (lo <= hi) P1.push_back({m, lo, hi});
    }
    
    if (L == 1) {
        adj[START-1].push_back({END_NODE, 1});
    }
    if (!P1.empty()) {
        int c = buildNode(P1, 1);
        if (c != -1) adj[START-1].push_back({c, 1});
    }
    
    // Save pre-merge state
    int n_before = n;
    vector<vector<pair<int,int>>> adj_before = adj;
    int START_before = START, END_before = END_NODE;
    
    mergePass();
    
    // Compact
    vector<int> remap(n + 1, 0);
    int nn = 0;
    for (int i = 1; i <= n; i++) {
        if (!deleted[i-1]) {
            nn++;
            remap[i] = nn;
        }
    }
    
    vector<vector<pair<int,int>>> newAdj(nn);
    for (int i = 1; i <= n; i++) {
        if (deleted[i-1]) continue;
        int ni = remap[i];
        for (auto& e : adj[i-1]) {
            newAdj[ni-1].push_back({remap[e.first], e.second});
        }
    }
    int newStart = remap[START];
    int newEnd = remap[END_NODE];
    
    if (nn <= n_before) {
        cout << nn << "\n";
        // Need to output START and END info? Original just prints n then adjacency
        // Match original format
        for (int i = 0; i < nn; i++) {
            cout << newAdj[i].size();
            for (auto& e : newAdj[i]) {
                cout << " " << e.first << " " << e.second;
            }
            cout << "\n";
        }
    } else {
        cout << n_before << "\n";
        for (int i = 0; i < n_before; i++) {
            cout << adj_before[i].size();
            for (auto& e : adj_before[i]) {
                cout << " " << e.first << " " << e.second;
            }
            cout << "\n";
        }
    }
    
    return 0;
}