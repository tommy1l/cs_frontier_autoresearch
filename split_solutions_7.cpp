#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

map<tuple<int,long long,long long>, int> memo;
int END_NODE;

int getState(int k, long long lo, long long hi) {
    if (lo > hi) return -1;
    if (k == 0) return END_NODE;
    auto key = make_tuple(k, lo, hi);
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    int node = newNode();
    memo[key] = node;
    long long half = 1LL << (k-1);
    long long lo0 = max(0LL, lo);
    long long hi0 = min(half - 1, hi);
    if (lo0 <= hi0) {
        int c = getState(k-1, lo0, hi0);
        if (c != -1) adj[node-1].push_back({c, 0});
    }
    long long lo1 = max(0LL, lo - half);
    long long hi1 = min(half - 1, hi - half);
    if (lo1 <= hi1) {
        int c = getState(k-1, lo1, hi1);
        if (c != -1) adj[node-1].push_back({c, 1});
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
    memo[make_tuple(0, 0LL, 0LL)] = END_NODE;
    
    for (int ell = ellL; ell <= ellR; ell++) {
        int T;
        if (ell == 1) {
            T = END_NODE;
        } else {
            int m = ell - 1;
            long long losuf, hisuf;
            long long pw = 1LL << m;
            if (ell > ellL && ell < ellR) {
                losuf = 0;
                hisuf = pw - 1;
            } else if (ell == ellL && ell == ellR) {
                losuf = L - pw;
                hisuf = R - pw;
            } else if (ell == ellL && ell < ellR) {
                losuf = L - pw;
                hisuf = pw - 1;
            } else {
                losuf = 0;
                hisuf = R - pw;
            }
            T = getState(m, losuf, hisuf);
        }
        if (T != -1) adj[START-1].push_back({T, 1});
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