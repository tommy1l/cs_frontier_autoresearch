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

    auto sigEmpty = [&](const Sig& s) {
        for (int r = 1; r < MAXR; r++) if (s[r].first <= s[r].second) return false;
        return true;
    };

    // Start signature: for each r >= 1, valid r-bit integers in [L, R] (already >= 2^(r-1) by MSB).
    Sig start_sig;
    for (int r = 0; r < MAXR; r++) start_sig[r] = {1, 0};
    for (int r = 1; r < MAXR; r++) {
        if (r < K_min || r > K_max) continue;
        long long a = max(L, 1LL << (r - 1));
        long long b = min(R, (1LL << r) - 1);
        if (a <= b) start_sig[r] = {a, b};
    }

    // computeChild: child[r] = {s : bit * 2^r + s in cur[r+1]} clipped to [0, 2^r - 1].
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

    // directAccept: bit is itself a valid 1-bit suffix from current state.
    auto directAccept = [&](const Sig& cur, int bit) {
        auto [lo, hi] = cur[1];
        return lo <= (long long)bit && (long long)bit <= hi;
    };

    map<Sig, int> stateId;
    vector<Sig> states;
    vector<vector<pair<int,int>>> edges;

    int START = 0, END = 1;

    states.push_back(start_sig);
    stateId[start_sig] = 0;
    edges.push_back({});

    Sig end_sig; for (int r = 0; r < MAXR; r++) end_sig[r] = {1, 0};
    states.push_back(end_sig);
    stateId[end_sig] = 1;
    edges.push_back({});

    auto getOrAdd = [&](const Sig& s, bool& isNew) -> int {
        isNew = false;
        if (sigEmpty(s)) return END;
        auto it = stateId.find(s);
        if (it != stateId.end()) return it->second;
        int id = states.size();
        stateId[s] = id;
        states.push_back(s);
        edges.push_back({});
        isNew = true;
        return id;
    };

    queue<int> bfs;
    bfs.push(START);

    while (!bfs.empty()) {
        int u = bfs.front(); bfs.pop();
        if (u == END) continue;
        Sig cur = states[u];
        for (int bit = 0; bit < 2; bit++) {
            if (directAccept(cur, bit)) {
                edges[u].push_back({END, bit});
            }
            Sig child = computeChild(cur, bit);
            if (!sigEmpty(child)) {
                bool isNew = false;
                int v = getOrAdd(child, isNew);
                edges[u].push_back({v, bit});
                if (isNew) {
                    bfs.push(v);
                }
            }
        }
    }

    int N = states.size();
    cout << N << "\n";
    for (int i = 0; i < N; i++) {
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
