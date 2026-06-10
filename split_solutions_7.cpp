#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;
vector<vector<tuple<int,long long,long long>>> nodeProfile;

int newNode() {
    n++;
    adj.push_back({});
    nodeProfile.push_back({});
    return n;
}

int START_NODE, END_NODE;
map<string, int> profMemo;

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
    
    // BFS depths from START
    vector<int> depth(n+1, -1);
    queue<int> q;
    depth[START_NODE] = 0;
    q.push(START_NODE);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto& e : adj[u-1]) {
            int v = e.first;
            if (depth[v] == -1) {
                depth[v] = depth[u] + 1;
                q.push(v);
            }
        }
    }
    
    vector<bool> deleted(n+1, false);
    vector<set<int>> mset(n+1);
    for (int u = 1; u <= n; u++) {
        for (auto& t : nodeProfile[u-1]) {
            mset[u].insert(get<0>(t));
        }
    }
    
    // Group by depth
    map<int, vector<int>> byDepth;
    for (int u = 1; u <= n; u++) {
        if (u == START_NODE || u == END_NODE) continue;
        if (depth[u] <= 0) continue;
        byDepth[depth[u]].push_back(u);
    }
    
    auto mergedOutdeg = [&](int u, int v) -> int {
        set<pair<int,int>> s;
        for (auto& e : adj[u-1]) s.insert(e);
        for (auto& e : adj[v-1]) s.insert(e);
        return (int)s.size();
    };
    
    auto disjointMset = [&](int u, int v) {
        if (mset[u].size() > mset[v].size()) swap(u, v);
        for (int m : mset[u]) if (mset[v].count(m)) return false;
        return true;
    };
    
    bool progress = true;
    while (progress) {
        progress = false;
        for (auto& kv : byDepth) {
            auto& vec = kv.second;
            // remove deleted
            vec.erase(remove_if(vec.begin(), vec.end(), [&](int x){ return deleted[x]; }), vec.end());
            
            bool found = false;
            for (int i = 0; i < (int)vec.size() && !found; i++) {
                int u = vec[i];
                if (deleted[u]) continue;
                for (int j = i+1; j < (int)vec.size() && !found; j++) {
                    int v = vec[j];
                    if (deleted[v]) continue;
                    if (depth[u] <= 0 || depth[v] <= 0) continue;
                    if (!disjointMset(u, v)) continue;
                    if (mergedOutdeg(u, v) > 200) continue;
                    
                    // Merge v into u
                    set<pair<int,int>> s;
                    for (auto& e : adj[u-1]) s.insert(e);
                    for (auto& e : adj[v-1]) s.insert(e);
                    adj[u-1].assign(s.begin(), s.end());
                    
                    for (auto& t : nodeProfile[v-1]) nodeProfile[u-1].push_back(t);
                    for (int m : mset[v]) mset[u].insert(m);
                    
                    // Rewrite predecessors
                    for (int p = 1; p <= n; p++) {
                        if (deleted[p]) continue;
                        bool touched = false;
                        for (auto& e : adj[p-1]) {
                            if (e.first == v) { e.first = u; touched = true; }
                        }
                        if (touched) {
                            set<pair<int,int>> ss;
                            for (auto& e : adj[p-1]) {
                                if (e.first == p) continue; // skip self loop
                                ss.insert(e);
                            }
                            adj[p-1].assign(ss.begin(), ss.end());
                        }
                    }
                    
                    deleted[v] = true;
                    adj[v-1].clear();
                    mset[v].clear();
                    nodeProfile[v-1].clear();
                    
                    found = true;
                    progress = true;
                }
            }
            if (found) break;
        }
    }
    
    // Compact ids
    vector<int> remap(n+1, 0);
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