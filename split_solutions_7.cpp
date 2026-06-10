#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;
int START_NODE, END_NODE;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

struct Trip { int m; long long lo, hi; };
map<string, int> profMemo;
vector<vector<Trip>> nodeProfile;

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
    if ((int)nodeProfile.size() < node) nodeProfile.resize(node);
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

vector<bool> deleted;

vector<int> topoOrder() {
    vector<int> indeg(n+1, 0);
    for (int u = 1; u <= n; u++) {
        if (deleted[u]) continue;
        for (auto& e : adj[u-1]) {
            if (!deleted[e.first]) indeg[e.first]++;
        }
    }
    queue<int> q;
    for (int u = 1; u <= n; u++) {
        if (!deleted[u] && indeg[u] == 0) q.push(u);
    }
    vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (auto& e : adj[u-1]) {
            if (deleted[e.first]) continue;
            if (--indeg[e.first] == 0) q.push(e.first);
        }
    }
    return order;
}

vector<vector<uint64_t>> computeReach() {
    int W = (n + 1 + 63) / 64;
    vector<vector<uint64_t>> reach(n+1, vector<uint64_t>(W, 0));
    vector<int> order = topoOrder();
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        for (auto& e : adj[u-1]) {
            int c = e.first;
            if (deleted[c]) continue;
            reach[u][c >> 6] |= (1ULL << (c & 63));
            for (int w = 0; w < W; w++) reach[u][w] |= reach[c][w];
        }
    }
    return reach;
}

long long totalPathCount(long long cap, int maxLen) {
    vector<vector<long long>> cnt(n+1, vector<long long>(maxLen+2, 0));
    cnt[END_NODE][0] = 1;
    vector<int> order = topoOrder();
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        if (u == END_NODE) continue;
        for (int k = 1; k <= maxLen+1; k++) {
            long long s = 0;
            for (auto& e : adj[u-1]) {
                int c = e.first;
                if (deleted[c]) continue;
                s += cnt[c][k-1];
                if (s > cap) { s = cap+1; break; }
            }
            cnt[u][k] = min(s, cap+1);
        }
    }
    long long total = 0;
    for (int k = 0; k <= maxLen+1; k++) {
        total += cnt[START_NODE][k];
        if (total > cap) return cap+1;
    }
    return total;
}

bool valueIntervalDisjoint(int u, int v) {
    // for each length m the value intervals must satisfy max(los) > min(his)
    map<int, vector<pair<long long,long long>>> byM;
    for (auto& t : nodeProfile[u-1]) byM[t.m].push_back({t.lo, t.hi});
    for (auto& t : nodeProfile[v-1]) byM[t.m].push_back({t.lo, t.hi});
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

int main() {
    long long L, R;
    cin >> L >> R;
    
    auto bitlen = [](long long x) {
        int b = 0;
        while (x) { b++; x >>= 1; }
        return b;
    };
    
    int ellL = bitlen(L), ellR = bitlen(R);
    
    START_NODE = newNode();
    END_NODE = newNode();
    nodeProfile.resize(n);
    
    vector<Trip> P1;
    for (int m = ellL - 1; m <= ellR - 1; m++) {
        if (m < 1) continue;
        long long pw = 1LL << m;
        long long lo = max(0LL, L - pw);
        long long hi = min(pw - 1, R - pw);
        if (lo <= hi) P1.push_back({m, lo, hi});
    }
    
    if (L == 1) {
        adj[START_NODE-1].push_back({END_NODE, 1});
    }
    if (!P1.empty()) {
        int c = buildNode(P1, 1);
        if (c != -1) adj[START_NODE-1].push_back({c, 1});
    }
    
    nodeProfile.resize(n+1);
    deleted.assign(n+1, false);
    
    long long target = R - L + 1;
    long long cap = 1LL << 30;
    int maxLen = ellR;
    
    bool progress = true;
    while (progress) {
        progress = false;
        auto reach = computeReach();
        for (int u = 1; u <= n && !progress; u++) {
            if (deleted[u] || u == START_NODE || u == END_NODE) continue;
            for (int v = u+1; v <= n && !progress; v++) {
                if (deleted[v] || v == START_NODE || v == END_NODE) continue;
                if (u-1 >= (int)nodeProfile.size() || v-1 >= (int)nodeProfile.size()) continue;
                if (nodeProfile[u-1].empty() || nodeProfile[v-1].empty()) continue;
                if (!valueIntervalDisjoint(u, v)) continue;
                if (reach[u][v>>6] & (1ULL << (v&63))) continue;
                if (reach[v][u>>6] & (1ULL << (u&63))) continue;
                
                set<pair<int,int>> merged;
                bool bad = false;
                for (auto& e : adj[u-1]) {
                    if (deleted[e.first]) { bad = true; break; }
                    if (e.first == u) { bad = true; break; }
                    merged.insert(e);
                }
                if (bad) continue;
                for (auto& e : adj[v-1]) {
                    if (deleted[e.first]) { bad = true; break; }
                    if (e.first == v) { bad = true; break; }
                    merged.insert(e);
                }
                if (bad) continue;
                if (merged.size() > 200) continue;
                
                // snapshot
                vector<pair<int,int>> snapU = adj[u-1];
                vector<pair<int,int>> snapV = adj[v-1];
                vector<bool> snapDel = deleted;
                vector<pair<int, vector<pair<int,int>>>> snapPred;
                for (int p = 1; p <= n; p++) {
                    if (deleted[p]) continue;
                    bool has = false;
                    for (auto& e : adj[p-1]) if (e.first == v) { has = true; break; }
                    if (has) snapPred.push_back({p, adj[p-1]});
                }
                
                // mutate
                adj[u-1].assign(merged.begin(), merged.end());
                for (auto& t : nodeProfile[v-1]) nodeProfile[u-1].push_back(t);
                
                for (auto& sp : snapPred) {
                    int p = sp.first;
                    set<pair<int,int>> ns;
                    for (auto& e : adj[p-1]) {
                        int tgt = (e.first == v) ? u : e.first;
                        if (tgt == p) continue;
                        ns.insert({tgt, e.second});
                    }
                    adj[p-1].assign(ns.begin(), ns.end());
                }
                
                deleted[v] = true;
                adj[v-1].clear();
                
                long long pc = totalPathCount(cap, maxLen);
                if (pc == target) {
                    progress = true;
                } else {
                    // rollback
                    adj[u-1] = snapU;
                    adj[v-1] = snapV;
                    deleted = snapDel;
                    for (auto& sp : snapPred) {
                        adj[sp.first - 1] = sp.second;
                    }
                    // restore nodeProfile[u-1]
                    int origSize = (int)nodeProfile[u-1].size() - (int)nodeProfile[v-1].size();
                    nodeProfile[u-1].resize(origSize);
                }
            }
        }
    }
    
    // compact
    vector<int> remap(n+1, 0);
    int newN = 0;
    for (int i = 1; i <= n; i++) {
        if (!deleted[i]) {
            newN++;
            remap[i] = newN;
        }
    }
    // ensure START is 1 and END is 2 if possible - keep order
    vector<vector<pair<int,int>>> newAdj(newN);
    for (int i = 1; i <= n; i++) {
        if (deleted[i]) continue;
        int ni = remap[i];
        for (auto& e : adj[i-1]) {
            newAdj[ni-1].push_back({remap[e.first], e.second});
        }
    }
    
    cout << newN << "\n";
    for (int i = 0; i < newN; i++) {
        cout << newAdj[i].size();
        for (auto& e : newAdj[i]) {
            cout << " " << e.first << " " << e.second;
        }
        cout << "\n";
    }
    
    return 0;
}