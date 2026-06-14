#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    int END = 0;
    int START = 1;
    int nodeCount = 2;
    vector<vector<pair<int,int>>> edges(2);

    // W chain: W(0) = END, W(j) -> W(j-1) via bit 0 and bit 1.
    vector<int> wNode;
    wNode.push_back(END);

    function<int(int)> getW = [&](int j) -> int {
        while ((int)wNode.size() <= j) {
            int id = nodeCount++;
            edges.push_back({});
            int below = wNode.back();
            edges[id].push_back({below, 0});
            edges[id].push_back({below, 1});
            wNode.push_back(id);
        }
        return wNode[j];
    };

    // length-atom (r, lo, hi): consume r more bits, value in [lo, hi].
    // Full-range (lo=0, hi=2^r-1) short-circuits to W(r) — shared suffix.
    map<tuple<int, long long, long long>, int> atomNode;

    function<int(int, long long, long long)> getAtom = [&](int r, long long lo, long long hi) -> int {
        if (lo > hi) return -1;
        if (r == 0) return END;
        if (lo == 0 && hi == (1LL << r) - 1) return getW(r);

        auto key = make_tuple(r, lo, hi);
        auto it = atomNode.find(key);
        if (it != atomNode.end()) return it->second;

        int id = nodeCount++;
        edges.push_back({});
        atomNode[key] = id;

        long long shift = 1LL << (r - 1);
        for (int b = 0; b < 2; b++) {
            long long b_shift = (long long)b * shift;
            long long nlo = max(0LL, lo - b_shift);
            long long nhi = min(shift - 1, hi - b_shift);
            int child = getAtom(r - 1, nlo, nhi);
            if (child != -1) {
                edges[id].push_back({child, b});
            }
        }
        return id;
    };

    for (int k = Kmin; k <= Kmax; k++) {
        long long L_k = max(L, 1LL << (k - 1));
        long long R_k = min(R, (1LL << k) - 1);
        if (L_k > R_k) continue;
        long long nlo = L_k - (1LL << (k - 1));
        long long nhi = R_k - (1LL << (k - 1));
        int atom = getAtom(k - 1, nlo, nhi);
        if (atom != -1) {
            edges[START].push_back({atom, 1});
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
