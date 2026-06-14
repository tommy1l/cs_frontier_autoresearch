#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int K_min = 0; { long long t = L; while (t > 0) { K_min++; t >>= 1; } }
    int K_max = 0; { long long t = R; while (t > 0) { K_max++; t >>= 1; } }

    const int MAXR = 25;
    using Sig = array<pair<long long, long long>, MAXR>;

    int nodeCount = 0;
    vector<vector<pair<int,int>>> edges;
    map<Sig, int> stateId;
    queue<pair<int, Sig>> bfs;

    int START = nodeCount++; edges.push_back({});
    int END = nodeCount++; edges.push_back({});

    vector<int> WC(K_max + 2, -1);
    WC[0] = END;
    function<int(int)> getWC = [&](int r) -> int {
        if (r == 0) return END;
        if (WC[r] != -1) return WC[r];
        int prev = getWC(r - 1);
        WC[r] = nodeCount++;
        edges.push_back({});
        edges[WC[r]].push_back({prev, 0});
        edges[WC[r]].push_back({prev, 1});
        return WC[r];
    };

    auto sigEmpty = [&](const Sig& s) {
        for (int r = 1; r < MAXR; r++) if (s[r].first <= s[r].second) return false;
        return true;
    };

    auto sigMatchesWC = [&](const Sig& s) -> int {
        int matched_r = -1;
        for (int r = 1; r < MAXR; r++) {
            if (s[r].first <= s[r].second) {
                long long full_hi = (1LL << r) - 1;
                if (s[r].first == 0 && s[r].second == full_hi && matched_r == -1) {
                    matched_r = r;
                } else {
                    return -1;
                }
            }
        }
        return matched_r;
    };

    auto getOrAdd = [&](const Sig& s) -> int {
        if (sigEmpty(s)) return END;
        int wc_r = sigMatchesWC(s);
        if (wc_r != -1) return getWC(wc_r);
        auto it = stateId.find(s);
        if (it != stateId.end()) return it->second;
        int id = nodeCount++;
        stateId[s] = id;
        edges.push_back({});
        bfs.push({id, s});
        return id;
    };

    auto computeChild = [&](const Sig& cur, int bit) -> Sig {
        Sig child;
        for (int r = 0; r < MAXR; r++) child[r] = {1, 0};
        for (int r_new = 1; r_new + 1 < MAXR; r_new++) {
            auto [lo, hi] = cur[r_new + 1];
            if (lo > hi) continue;
            long long shift = 1LL << r_new;
            long long b_shift = (long long)bit * shift;
            long long nlo = max(0LL, lo - b_shift);
            long long nhi = min(shift - 1, hi - b_shift);
            if (nlo <= nhi) child[r_new] = {nlo, nhi};
        }
        return child;
    };

    auto directAccept = [&](const Sig& cur, int bit) {
        auto [lo, hi] = cur[1];
        return lo <= (long long)bit && (long long)bit <= hi;
    };

    auto processState = [&](int u, const Sig& cur) {
        for (int bit = 0; bit < 2; bit++) {
            if (directAccept(cur, bit)) {
                edges[u].push_back({END, bit});
            }
            Sig child = computeChild(cur, bit);
            if (!sigEmpty(child)) {
                int v = getOrAdd(child);
                edges[u].push_back({v, bit});
            }
        }
    };

    Sig boundary_sig;
    for (int r = 0; r < MAXR; r++) boundary_sig[r] = {1, 0};
    bool has_boundary = false;

    for (int K = K_min; K <= K_max; K++) {
        long long a = max(L, 1LL << (K - 1));
        long long b = min(R, (1LL << K) - 1);
        if (a > b) continue;
        long long resid_lo = a - (1LL << (K - 1));
        long long resid_hi = b - (1LL << (K - 1));

        if (K == 1) {
            edges[START].push_back({END, 1});
            continue;
        }
        long long full_hi = (1LL << (K - 1)) - 1;
        bool is_full = (resid_lo == 0 && resid_hi == full_hi);

        if (is_full) {
            edges[START].push_back({getWC(K - 1), 1});
        } else {
            boundary_sig[K - 1] = {resid_lo, resid_hi};
            has_boundary = true;
        }
    }

    if (has_boundary) {
        int bid = getOrAdd(boundary_sig);
        edges[START].push_back({bid, 1});
    }

    while (!bfs.empty()) {
        auto [u, sig] = bfs.front();
        bfs.pop();
        processState(u, sig);
    }

    int N = nodeCount;
    vector<int> remap(N, -1);
    remap[START] = 0;
    remap[END] = N - 1;
    int idx = 1;
    for (int i = 0; i < N; i++) {
        if (i == START || i == END) continue;
        remap[i] = idx++;
    }

    vector<vector<pair<int,int>>> newEdges(N);
    for (int i = 0; i < N; i++) {
        for (auto& [a, w] : edges[i]) {
            newEdges[remap[i]].push_back({remap[a], w});
        }
    }

    cout << N << "\n";
    for (int i = 0; i < N; i++) {
        cout << newEdges[i].size();
        for (auto& [a, w] : newEdges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
