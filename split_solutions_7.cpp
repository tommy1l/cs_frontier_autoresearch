#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;
int START_NODE;
int END_NODE;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

map<string, int> profMemo;
vector<vector<tuple<int,long long,long long>>> nodeProfile;

struct Trip { int m; long long lo, hi; };

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
    nodeProfile.resize(n);
    for (auto& t : profile) nodeProfile[node-1].push_back({t.m, t.lo, t.hi});
    
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

bool valueIntervalDisjoint(int u, int v) {
    map<int, vector<pair<long long,long long>>> byM_u, byM_v;
    for (auto& t : nodeProfile[u-1]) byM_u[get<0>(t)].push_back({get<1>(t), get<2>(t)});
    for (auto& t : nodeProfile[v-1]) byM_v[get<0>(t)].push_back({get<1>(t), get<2>(t)});
    for (auto& [m, vu] : byM_u) {
        auto it = byM_v.find(m);
        if (it == byM_v.end()) continue;
        auto& vv = it->second;
        long long maxLo = LLONG_MIN, minHi = LLONG_MAX;
        for (auto& p : vu) { maxLo = max(maxLo, p.first); minHi = min(minHi, p.second); }
        for (auto& p : vv) { maxLo = max(maxLo, p.first); minHi = min(minHi, p.second); }
        if (maxLo <= minHi) return false;
    }
    return true;
}

vector<vector<uint64_t>> recomputeReach(vector<bool>& deleted) {
    int W = ((n + 1) + 63) / 64;
    vector<vector<uint64_t>> reach(n + 1, vector<uint64_t>(W, 0));
    
    vector<int> indeg(n + 1, 0);
    for (int u = 1; u <= n; u++) {
        if (deleted[u]) continue;
        for (auto& e : adj[u-1]) {
            if (!deleted[e.first]) indeg[e.first]++;
        }
    }
    queue<int> q;
    for (int u = 1; u <= n; u++) if (!deleted[u] && indeg[u] == 0) q.push(u);
    vector<int> topo;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        topo.push_back(u);
        for (auto& e : adj[u-1]) {
            if (deleted[e.first]) continue;
            if (--indeg[e.first] == 0) q.push(e.first);
        }
    }
    
    for (int i = (int)topo.size() - 1; i >= 0; i--) {
        int u = topo[i];
        for (int w = 0; w < W; w++) reach[u][w] = 0;
        for (auto& e : adj[u-1]) {
            int c = e.first;
            if (deleted[c]) continue;
            reach[u][c >> 6] |= (1ULL << (c & 63));
            for (int w = 0; w < W; w++) reach[u][w] |= reach[c][w];
        }
    }
    return reach;
}

bool isMergeable(int u, int v, vector<bool>& deleted, vector<vector<uint64_t>>& reach) {
    if (deleted[u] || deleted[v]) return false;
    if (u == START_NODE || u == END_NODE || v == START_NODE || v == END_NODE) return false;
    if (!valueIntervalDisjoint(u, v)) return false;
    set<pair<int,int>> merged;
    for (auto& e : adj[u-1]) {
        if (e.first == u || e.first == v) return false;
        if (deleted[e.first]) return false;
        merged.insert(e);
    }
    for (auto& e : adj[v-1]) {
        if (e.first == u || e.first == v) return false;
        if (deleted[e.first]) return false;
        merged.insert(e);
    }
    if ((int)merged.size() > 200) return false;
    if ((reach[u][v >> 6] >> (v & 63)) & 1ULL) return false;
    if ((reach[v][u >> 6] >> (u & 63)) & 1ULL) return false;
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
    
    nodeProfile.resize(n);
    
    vector<bool> deleted(n + 1, false);
    
    while (true) {
        bool progress = false;
        auto reach = recomputeReach(deleted);
        for (int u = 1; u <= n && !progress; u++) {
            if (deleted[u]) continue;
            for (int v = u + 1; v <= n && !progress; v++) {
                if (deleted[v]) continue;
                if (isMergeable(u, v, deleted, reach)) {
                    set<pair<int,int>> merged;
                    for (auto& e : adj[u-1]) merged.insert(e);
                    for (auto& e : adj[v-1]) merged.insert(e);
                    adj[u-1].assign(merged.begin(), merged.end());
                    for (auto& t : nodeProfile[v-1]) nodeProfile[u-1].push_back(t);
                    for (int p = 1; p <= n; p++) {
                        if (deleted[p]) continue;
                        if (p == v) continue;
                        bool touched = false;
                        for (auto& e : adj[p-1]) {
                            if (e.first == v) { e.first = u; touched = true; }
                        }
                        if (touched) {
                            set<pair<int,int>> s(adj[p-1].begin(), adj[p-1].end());
                            adj[p-1].assign(s.begin(), s.end());
                        }
                    }
                    deleted[v] = true;
                    adj[v-1].clear();
                    progress = true;
                }
            }
        }
        if (!progress) break;
    }
    
    vector<int> remap(n + 1, 0);
    int newN = 0;
    for (int i = 1; i <= n; i++) {
        if (!deleted[i]) {
            newN++;
            remap[i] = newN;
        }
    }
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