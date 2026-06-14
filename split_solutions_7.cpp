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

    int n = nodeCount;
    int END = W(0);

    // DFA minimization (Moore): collapse nodes with identical future-language.
    vector<int> cls(n, 1);
    cls[END] = 0;

    while (true) {
        map<tuple<int, vector<pair<int,int>>>, int> sigMap;
        vector<int> ncls(n);
        int newK = 0;
        for (int u = 0; u < n; u++) {
            vector<pair<int,int>> sig;
            for (auto& [t, b] : edges[u]) {
                sig.push_back({b, cls[t]});
            }
            sort(sig.begin(), sig.end());
            auto key = make_tuple(cls[u], sig);
            auto it = sigMap.find(key);
            if (it == sigMap.end()) {
                sigMap[key] = newK;
                ncls[u] = newK;
                newK++;
            } else {
                ncls[u] = it->second;
            }
        }
        int oldK = *max_element(cls.begin(), cls.end()) + 1;
        if (newK == oldK) break;
        cls = ncls;
    }

    int newN = *max_element(cls.begin(), cls.end()) + 1;
    vector<int> repr(newN, -1);
    for (int u = 0; u < n; u++) if (repr[cls[u]] == -1) repr[cls[u]] = u;

    int startCls = cls[START];
    vector<int> remap(newN);
    remap[startCls] = 0;
    int idx = 1;
    for (int c = 0; c < newN; c++) {
        if (c != startCls) remap[c] = idx++;
    }

    vector<vector<pair<int,int>>> finalEdges(newN);
    for (int u = 0; u < n; u++) {
        if (repr[cls[u]] != u) continue;
        for (auto& [t, b] : edges[u]) {
            finalEdges[remap[cls[u]]].push_back({remap[cls[t]], b});
        }
    }

    cout << newN << "\n";
    for (int i = 0; i < newN; i++) {
        cout << finalEdges[i].size();
        for (auto& [a, w] : finalEdges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
