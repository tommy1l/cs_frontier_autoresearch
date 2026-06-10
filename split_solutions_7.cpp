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
    
    vector<bool> deleted(n+1, false);
    
    auto valueIntervalDisjoint = [&](int u, int v) -> bool {
        map<int, vector<pair<long long,long long>>> mu, mv;
        for (auto& t : nodeProfile[u-1]) mu[get<0>(t)].push_back({get<1>(t), get<2>(t)});
        for (auto& t : nodeProfile[v-1]) mv[get<0>(t)].push_back({get<1>(t), get<2>(t)});
        for (auto& kv : mu) {
            int m = kv.first;
            auto itv = mv.find(m);
            if (itv == mv.end()) continue;
            long long lo_u = LLONG_MAX, hi_u = LLONG_MIN;
            for (auto& p : kv.second) { lo_u = min(lo_u, p.first); hi_u = max(hi_u, p.second); }
            long long lo_v = LLONG_MAX, hi_v = LLONG_MIN;
            for (auto& p : itv->second) { lo_v = min(lo_v, p.first); hi_v = max(hi_v, p.second); }
            if (max(lo_u, lo_v) <= min(hi_u, hi_v)) return false;
        }
        return true;
    };
    
    auto mergedOutdegSet = [&](int u, int v) -> int {
        set<pair<int,int>> s;
        for (auto& e : adj[u-1]) s.insert(e);
        for (auto& e : adj[v-1]) s.insert(e);
        return (int)s.size();
    };
    
    auto checkAcyclic = [&]() -> bool {
        int liveCount = 0;
        vector<int> indeg(n+1, 0);
        for (int p = 1; p <= n; p++) {
            if (deleted[p]) continue;
            liveCount++;
        }
        for (int p = 1; p <= n; p++) {
            if (deleted[p]) continue;
            for (auto& e : adj[p-1]) {
                if (deleted[e.first]) continue;
                indeg[e.first]++;
            }
        }
        queue<int> q;
        for (int p = 1; p <= n; p++) {
            if (deleted[p]) continue;
            if (indeg[p] == 0) q.push(p);
        }
        int popped = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            popped++;
            for (auto& e : adj[u-1]) {
                if (deleted[e.first]) continue;
                if (--indeg[e.first] == 0) q.push(e.first);
            }
        }
        return popped == liveCount;
    };
    
    bool progress = true;
    while (progress) {
        progress = false;
        for (int u = 1; u <= n && !progress; u++) {
            if (deleted[u]) continue;
            if (u == START_NODE || u == END_NODE) continue;
            for (int v = u+1; v <= n && !progress; v++) {
                if (deleted[v]) continue;
                if (v == START_NODE || v == END_NODE) continue;
                
                if (!valueIntervalDisjoint(u, v)) continue;
                if (mergedOutdegSet(u, v) > 200) continue;
                
                // Snapshot
                vector<pair<int,int>> snap_adj_u = adj[u-1];
                vector<pair<int,int>> snap_adj_v = adj[v-1];
                vector<tuple<int,long long,long long>> snap_prof_u = nodeProfile[u-1];
                vector<bool> snap_deleted = deleted;
                map<int, vector<pair<int,int>>> snap_preds;
                
                for (int p = 1; p <= n; p++) {
                    if (deleted[p]) continue;
                    for (auto& e : adj[p-1]) {
                        if (e.first == v) {
                            snap_preds[p] = adj[p-1];
                            break;
                        }
                    }
                }
                
                // Mutate
                {
                    set<pair<int,int>> s;
                    for (auto& e : adj[u-1]) s.insert(e);
                    for (auto& e : adj[v-1]) s.insert(e);
                    adj[u-1].assign(s.begin(), s.end());
                }
                for (auto& t : nodeProfile[v-1]) nodeProfile[u-1].push_back(t);
                
                for (int p = 1; p <= n; p++) {
                    if (deleted[p]) continue;
                    bool touched = false;
                    for (auto& e : adj[p-1]) {
                        if (e.first == v) { e.first = u; touched = true; }
                    }
                    if (touched) {
                        set<pair<int,int>> ss;
                        for (auto& e : adj[p-1]) ss.insert(e);
                        adj[p-1].assign(ss.begin(), ss.end());
                    }
                }
                
                deleted[v] = true;
                adj[v-1].clear();
                
                if (checkAcyclic()) {
                    progress = true;
                } else {
                    // Rollback
                    adj[u-1] = snap_adj_u;
                    adj[v-1] = snap_adj_v;
                    nodeProfile[u-1] = snap_prof_u;
                    deleted = snap_deleted;
                    for (auto& kv : snap_preds) {
                        adj[kv.first - 1] = kv.second;
                    }
                }
            }
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