#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

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
    int firstSuffix = 2 + maxK;
    int nodeCount = firstSuffix;
    vector<vector<pair<int,int>>> edges(nodeCount);

    for (int i = 1; i <= maxK; i++) {
        edges[W(i)].push_back({W(i-1), 0});
        edges[W(i)].push_back({W(i-1), 1});
    }

    // Suffix-DAWG: state keyed by (suffix_bits, suffix_len, k).
    // Hash-cons backwards from W(k) toward START so pieces sharing the
    // trailing bits of their prefix (just before the wildcard chain) and
    // their wildcard count k merge their tail paths.
    map<tuple<long long, int, int>, int> sufStateId;
    function<int(long long, int, int)> getSuf = [&](long long s, int len, int k) -> int {
        if (len == 0) return W(k);
        auto key = make_tuple(s, len, k);
        auto it = sufStateId.find(key);
        if (it != sufStateId.end()) return it->second;
        int id = nodeCount++;
        edges.push_back({});
        sufStateId[key] = id;
        int bit = (int)((s >> (len - 1)) & 1);
        long long rest = s & ((1LL << (len - 1)) - 1);
        int child = getSuf(rest, len - 1, k);
        edges[id].push_back({child, bit});
        return id;
    };

    for (auto& [prefix, pBits, k] : pieces) {
        int leadBit = (int)((prefix >> (pBits - 1)) & 1);
        if (pBits == 1) {
            edges[START].push_back({W(k), leadBit});
        } else {
            long long rest = prefix & ((1LL << (pBits - 1)) - 1);
            int child = getSuf(rest, pBits - 1, k);
            edges[START].push_back({child, leadBit});
        }
    }

    cout << nodeCount << "\n";
    for (int i = 0; i < nodeCount; i++) {
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
