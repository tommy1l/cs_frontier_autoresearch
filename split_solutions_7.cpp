#include <bits/stdc++.h>
using namespace std;

struct DAG {
    int n;
    vector<vector<pair<int, int>>> edges;
};

DAG buildE370508(long long L, long long R) {
    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    int END = 0;
    int START = 1;
    int nodeCount = 2;
    vector<vector<pair<int,int>>> edges(2);

    map<vector<tuple<int, long long, long long>>, int> stateMap;

    function<int(long long, int)> getState = [&](long long v, int d) -> int {
        int rMin = max(1, Kmin - d);
        int rMax = Kmax - d;
        vector<tuple<int, long long, long long>> sig;
        for (int r = rMin; r <= rMax; r++) {
            long long mul = 1LL << r;
            long long lo_r = max(0LL, L - v * mul);
            long long hi_r = min(mul - 1, R - v * mul);
            if (lo_r <= hi_r) {
                sig.push_back({r, lo_r, hi_r});
            }
        }
        if (sig.empty()) return -1;

        auto it = stateMap.find(sig);
        if (it != stateMap.end()) return it->second;

        int id = nodeCount++;
        edges.push_back({});
        stateMap[sig] = id;

        for (int b = 0; b < 2; b++) {
            long long vp = 2 * v + b;
            int dp = d + 1;
            if (L <= vp && vp <= R && Kmin <= dp && dp <= Kmax) {
                edges[id].push_back({END, b});
            }
            if (dp < Kmax) {
                int child = getState(vp, dp);
                if (child != -1) {
                    edges[id].push_back({child, b});
                }
            }
        }
        return id;
    };

    if (L <= 1 && 1 <= R) {
        edges[START].push_back({END, 1});
    }
    if (Kmax >= 2) {
        int s = getState(1, 1);
        if (s != -1) {
            edges[START].push_back({s, 1});
        }
    }

    return {nodeCount, edges};
}

DAG buildCab21ec(long long L, long long R) {
    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    vector<tuple<long long, int, int>> pieces;

    auto decomposeLen = [&](long long lo, long long hi, int totalBits) {
        long long a = lo;
        while (a <= hi) {
            int j = 0;
            while (true) {
                long long m = (1LL << (j + 1)) - 1;
                if ((a & m) != 0) break;
                if (a + (1LL << (j + 1)) - 1 > hi) break;
                j++;
            }
            int pBits = totalBits - j;
            long long p = a >> j;
            pieces.push_back({p, pBits, j});
            a += (1LL << j);
        }
    };

    for (int k = Kmin; k <= Kmax; k++) {
        long long lo = max(L, 1LL << (k - 1));
        long long hi = min(R, (1LL << k) - 1);
        if (lo <= hi) decomposeLen(lo, hi, k);
    }

    int maxK = 0;
    for (auto& [p, pb, k] : pieces) maxK = max(maxK, k);

    int START = 0;
    auto W = [&](int k) { return 1 + k; };
    int firstTrie = 2 + maxK;
    int nodeCount = firstTrie;

    vector<vector<pair<int,int>>> edges(nodeCount);

    for (int i = 1; i <= maxK; i++) {
        edges[W(i)].push_back({W(i-1), 0});
        edges[W(i)].push_back({W(i-1), 1});
    }

    map<pair<int,int>, int> trieChild;

    for (auto& [prefix, pBits, k] : pieces) {
        int cur = START;
        for (int d = pBits - 1; d >= 0; d--) {
            int bit = (int)((prefix >> d) & 1);
            if (d == 0) {
                edges[cur].push_back({W(k), bit});
            } else {
                auto it = trieChild.find({cur, bit});
                int nxt;
                if (it == trieChild.end()) {
                    nxt = nodeCount++;
                    edges.push_back({});
                    trieChild[{cur, bit}] = nxt;
                    edges[cur].push_back({nxt, bit});
                } else {
                    nxt = it->second;
                }
                cur = nxt;
            }
        }
    }

    return {nodeCount, edges};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    DAG d1 = buildE370508(L, R);
    DAG d2 = buildCab21ec(L, R);

    DAG& chosen = (d1.n <= d2.n) ? d1 : d2;

    cout << chosen.n << "\n";
    for (int i = 0; i < chosen.n; i++) {
        cout << chosen.edges[i].size();
        for (auto& [a, w] : chosen.edges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
