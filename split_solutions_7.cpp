#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;
vector<vector<tuple<int,long long,long long>>> nodeProfile;
vector<vector<tuple<int,long long,long long>>> prefixProfile;

int newNode() {
    n++;
    adj.push_back({});
    nodeProfile.push_back({});
    prefixProfile.push_back({});
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

// normalize a profile: group by length d, union intervals
void normalizeProfile(vector<tuple<int,long long,long long>>& P) {
    map<int, vector<pair<long long,long long>>> byD;
    for (auto& t : P) {
        byD[get<0>(t)].push_back({get<1>(t), get<2>(t)});
    }
    P.clear();
    for (auto& kv : byD) {
        auto& vec = kv.second;
        sort(vec.begin(), vec.end());
        vector<pair<long long,long long>> merged;
        for (auto& iv : vec) {
            if (!merged.empty() && iv.first <= merged.back().second + 1) {
                merged.back().second = max(merged.back().second, iv.second);
            } else {
                merged.push_back(iv);
            }
        }
        for (auto& iv : merged) P.push_back({kv.first, iv.first, iv.second});
    }
}

bool profilesDisjoint(const vector<tuple<int,long long,long long>>& A,
                      const vector<tuple<int,long long,long long>>& B) {
    map<int, vector<pair<long long,long long>>> ma;
    for (auto& t : A) ma[get<0>(t)].push_back({get<1>(t), get<2>(t)});
    for (auto& t : B) {
        int d = get<0>(t);
        auto it = ma.find(d);
        if (it == ma.end()) continue;
        long long lo = get<1>(t), hi = get<2>(t);
        for (auto& iv : it->second) {
            if (iv.first <= hi && lo <= iv.second) return false;
        }
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
    
    vector<bool> deleted(n+1, false);
    vector<set<int>> mset(n+1);
    for (int u = 1; u <= n; u++) {
        for (auto& t : nodeProfile[u-1]) {
            mset[u].insert(get<0>(t));
        }
    }
    
    auto computePrefixProfiles = [&]() {
        for (int i = 0; i < n; i++) prefixProfile[i].clear();
        prefixProfile[START_NODE-1].push_back({0, 0, 0});
        
        // toposort
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
        vector<int> topo;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            topo.push_back(u);
            for (auto& e : adj[u-1]) {
                if (deleted[e.first]) continue;
                if (--indeg[e.first] == 0) q.push(e.first);
            }
        }
        for (int u : topo) {
            normalizeProfile(prefixProfile[u-1]);
            for (auto& e : adj[u-1]) {
                int c = e.first;
                int b = e.second;
                if (deleted[c]) continue;
                for (auto& t : prefixProfile[u-1]) {
                    int d = get<0>(t);
                    long long lo = get<1>(t), hi = get<2>(t);
                    long long nlo = 2*lo + b;
                    long long nhi = 2*hi + b;
                    if (nlo < 0) nlo = 0;
                    if (nhi < 0) continue;
                    prefixProfile[c-1].push_back({d+1, nlo, nhi});
                }
            }
        }
        for (int u = 1; u <= n; u++) {
            if (!deleted[u]) normalizeProfile(prefixProfile[u-1]);
        }
    };
    
    // reach bitsets
    auto computeReach = [&](vector<vector<uint64_t>>& reach) {
        int W = (n + 63) / 64;
        reach.assign(n+1, vector<uint64_t>(W, 0));
        // reverse topo
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
        vector<int> topo;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            topo.push_back(u);
            for (auto& e : adj[u-1]) {
                if (deleted[e.first]) continue;
                if (--indeg[e.first] == 0) q.push(e.first);
            }
        }
        for (int i = (int)topo.size()-1; i >= 0; i--) {
            int u = topo[i];
            reach[u][(u-1)/64] |= (1ULL << ((u-1)%64));
            for (auto& e : adj[u-1]) {
                if (deleted[e.first]) continue;
                int c = e.first;
                for (int k = 0; k < W; k++) reach[u][k] |= reach[c][k];
            }
        }
    };
    
    auto mergedOutdeg = [&](int u, int v) -> int {
        set<pair<int,int>> s;
        for (auto& e : adj[u-1]) s.insert(e);
        for (auto& e : adj[v-1]) s.insert(e);
        return (int)s.size();
    };
    
    computePrefixProfiles();
    vector<vector<uint64_t>> reach;
    computeReach(reach);
    
    bool progress = true;
    while (progress) {
        progress = false;
        bool found = false;
        for (int u = 1; u <= n && !found; u++) {
            if (deleted[u] || u == START_NODE || u == END_NODE) continue;
            for (int v = u+1; v <= n && !found; v++) {
                if (deleted[v] || v == START_NODE || v == END_NODE) continue;
                
                // suffix profile disjoint
                vector<tuple<int,long long,long long>> sufU, sufV;
                for (auto& t : nodeProfile[u-1]) sufU.push_back(t);
                for (auto& t : nodeProfile[v-1]) sufV.push_back(t);
                if (!profilesDisjoint(sufU, sufV)) continue;
                
                // prefix profile disjoint
                if (!profilesDisjoint(prefixProfile[u-1], prefixProfile[v-1])) continue;
                
                if (mergedOutdeg(u, v) > 200) continue;
                
                // cycle check
                if (reach[u][(v-1)/64] & (1ULL << ((v-1)%64))) continue;
                if (reach[v][(u-1)/64] & (1ULL << ((u-1)%64))) continue;
                
                // Merge v into u
                set<pair<int,int>> s;
                for (auto& e : adj[u-1]) s.insert(e);
                for (auto& e : adj[v-1]) s.insert(e);
                adj[u-1].assign(s.begin(), s.end());
                
                for (auto& t : nodeProfile[v-1]) nodeProfile[u-1].push_back(t);
                for (auto& t : prefixProfile[v-1]) prefixProfile[u-1].push_back(t);
                normalizeProfile(prefixProfile[u-1]);
                for (int m : mset[v]) mset[u].insert(m);
                
                for (int p = 1; p <= n; p++) {
                    if (deleted[p]) continue;
                    bool touched = false;
                    for (auto& e : adj[p-1]) {
                        if (e.first == v) { e.first = u; touched = true; }
                    }
                    if (touched) {
                        set<pair<int,int>> ss;
                        for (auto& e : adj[p-1]) {
                            if (e.first == p) continue;
                            ss.insert(e);
                        }
                        adj[p-1].assign(ss.begin(), ss.end());
                    }
                }
                
                deleted[v] = true;
                adj[v-1].clear();
                mset[v].clear();
                nodeProfile[v-1].clear();
                prefixProfile[v-1].clear();
                
                found = true;
                progress = true;
                
                computePrefixProfiles();
                computeReach(reach);
            }
        }
    }
    
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