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

    // Multi-length DFA: state(v, d) keyed by future-language signature
    // (list of (r, lo_r, hi_r) for r in [max(1, Kmin-d), Kmax-d]).
    // Predecessor emits bit-b -> END for length-(d+1) acceptance AND
    // bit-b -> state(2v+b, d+1) for extension. Same-bit multi-edges allowed
    // because different total lengths => unique paths.
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
            // length-(d+1) acceptance: emit bit-b -> END
            if (L <= vp && vp <= R && Kmin <= dp && dp <= Kmax) {
                edges[id].push_back({END, b});
            }
            // extend to deeper state for longer integers
            if (dp < Kmax) {
                int child = getState(vp, dp);
                if (child != -1) {
                    edges[id].push_back({child, b});
                }
            }
        }
        return id;
    };

    // START emits bit-1 -> END (length 1, integer 1) AND bit-1 -> state(1, 1).
    if (L <= 1 && 1 <= R) {
        edges[START].push_back({END, 1});
    }
    if (Kmax >= 2) {
        int s = getState(1, 1);
        if (s != -1) {
            edges[START].push_back({s, 1});
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
