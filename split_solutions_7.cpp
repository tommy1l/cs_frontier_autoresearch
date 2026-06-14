#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    // Canonical pieces: (prefix, prefixBits, wildcardCount).
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
    auto W = [&](int k) { return 1 + k; };  // W(0)=1 (END), W(i)=1+i
    int nextNode = 2 + maxK;
    vector<vector<pair<int,int>>> edges(nextNode);

    for (int i = 1; i <= maxK; i++) {
        edges[W(i)].push_back({W(i-1), 0});
        edges[W(i)].push_back({W(i-1), 1});
    }

    // Shared-suffix axis: each piece's prefix is emitted via states keyed by
    // (suffix-of-prefix-to-emit, k_wildcards). Two pieces share state if same
    // (suffix, k). Final state with empty suffix is W(k). Each state has ONE
    // outgoing edge (the next bit is determined by the suffix).
    map<tuple<int,long long,int>, int> stateMap;

    function<int(int, long long, int)> getState = [&](int sBits, long long sVal, int k) -> int {
        if (sBits == 0) return W(k);
        auto key = make_tuple(sBits, sVal, k);
        auto it = stateMap.find(key);
        if (it != stateMap.end()) return it->second;
        int id = nextNode++;
        edges.push_back({});
        stateMap[key] = id;
        int bit = (int)((sVal >> (sBits - 1)) & 1);
        long long restVal = sVal & ((1LL << (sBits - 1)) - 1);
        int child = getState(sBits - 1, restVal, k);
        edges[id].push_back({child, bit});
        return id;
    };

    for (auto& [prefix, pBits, k] : pieces) {
        int bit0 = (int)((prefix >> (pBits - 1)) & 1);
        long long restVal = prefix & ((1LL << (pBits - 1)) - 1);
        int child = getState(pBits - 1, restVal, k);
        edges[START].push_back({child, bit0});
    }

    cout << nextNode << "\n";
    for (int i = 0; i < nextNode; i++) {
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
