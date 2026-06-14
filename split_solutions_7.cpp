#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    int END_NODE = 0;
    int START = 1;
    int nodeCount = 2;
    vector<vector<pair<int,int>>> edges(2);

    // W chain: W[k] = node from which any k-bit string reaches END.
    // W[0] = END. W[k] -bit-b-> W[k-1].
    vector<int> W(Kmax + 2, -1);
    W[0] = END_NODE;
    function<int(int)> getW = [&](int k) -> int {
        if (k < 0) return -1;
        if (W[k] != -1) return W[k];
        int id = nodeCount++;
        edges.push_back({});
        W[k] = id;
        int prev = getW(k - 1);
        edges[id].push_back({prev, 0});
        edges[id].push_back({prev, 1});
        return id;
    };

    map<vector<tuple<int, long long, long long>>, int> stateMap;

    auto computeSig = [&](long long v, int d, vector<tuple<int, long long, long long>>& sig) {
        int rMin = max(1, Kmin - d);
        int rMax = Kmax - d;
        for (int r = rMin; r <= rMax; r++) {
            long long mul = 1LL << r;
            long long lo_r = max(0LL, L - v * mul);
            long long hi_r = min(mul - 1, R - v * mul);
            if (lo_r <= hi_r) {
                sig.push_back({r, lo_r, hi_r});
            }
        }
    };

    auto isFullyWild = [&](const vector<tuple<int, long long, long long>>& sig) {
        if (sig.empty()) return false;
        for (auto& [r, lo, hi] : sig) {
            long long mul = 1LL << r;
            if (lo != 0 || hi != mul - 1) return false;
        }
        return true;
    };

    function<int(long long, int)> getState;

    auto emitTransition = [&](int id, int b, long long vp, int dp) {
        if (L <= vp && vp <= R && Kmin <= dp && dp <= Kmax) {
            edges[id].push_back({END_NODE, b});
        }
        if (dp < Kmax) {
            vector<tuple<int, long long, long long>> childSig;
            computeSig(vp, dp, childSig);
            if (childSig.empty()) return;
            if (isFullyWild(childSig)) {
                for (auto& [r, lo, hi] : childSig) {
                    int target = getW(r - 1);
                    edges[id].push_back({target, b});
                }
            } else {
                int child = getState(vp, dp);
                if (child != -1) {
                    edges[id].push_back({child, b});
                }
            }
        }
    };

    getState = [&](long long v, int d) -> int {
        vector<tuple<int, long long, long long>> sig;
        computeSig(v, d, sig);
        if (sig.empty()) return -1;

        auto it = stateMap.find(sig);
        if (it != stateMap.end()) return it->second;

        int id = nodeCount++;
        edges.push_back({});
        stateMap[sig] = id;

        if (isFullyWild(sig)) {
            for (auto& [r, lo, hi] : sig) {
                int target = getW(r - 1);
                edges[id].push_back({target, 0});
                edges[id].push_back({target, 1});
            }
            return id;
        }

        for (int b = 0; b < 2; b++) {
            long long vp = 2 * v + b;
            int dp = d + 1;
            emitTransition(id, b, vp, dp);
        }
        return id;
    };

    if (L <= 1 && 1 <= R) {
        edges[START].push_back({END_NODE, 1});
    }
    if (Kmax >= 2) {
        long long vp = 1;
        int dp = 1;
        vector<tuple<int, long long, long long>> childSig;
        computeSig(vp, dp, childSig);
        if (!childSig.empty()) {
            if (isFullyWild(childSig)) {
                for (auto& [r, lo, hi] : childSig) {
                    int target = getW(r - 1);
                    edges[START].push_back({target, 1});
                }
            } else {
                int s = getState(vp, dp);
                if (s != -1) {
                    edges[START].push_back({s, 1});
                }
            }
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
