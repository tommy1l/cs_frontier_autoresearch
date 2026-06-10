#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

int END_NODE;
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
    
    int START = newNode();
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
        adj[START-1].push_back({END_NODE, 1});
    }
    if (!P1.empty()) {
        int c = buildNode(P1, 1);
        if (c != -1) adj[START-1].push_back({c, 1});
    }
    
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << adj[i].size();
        for (auto& e : adj[i]) {
            cout << " " << e.first << " " << e.second;
        }
        cout << "\n";
    }
    
    return 0;
}