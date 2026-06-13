#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int subtask, n;
    cin >> subtask >> n;

    auto outputGuess = [&](const vector<int>& perm) {
        cout << -1;
        for (int x : perm) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    };

    if (n <= 1000) {
        long long L = 2LL * n * (n - 1);
        cout << L;
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                cout << ' ' << i << ' ' << j << ' ' << j << ' ' << i;
            }
        }
        cout << '\n';
        cout.flush();

        vector<int> resp(L);
        for (long long k = 0; k < L; k++) cin >> resp[k];

        vector<vector<int>> adj(n + 1);
        long long idx = 0;
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                if (resp[idx + 1] == 1) {
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                }
                idx += 4;
            }
        }

        vector<int> perm;
        perm.reserve(n);
        perm.push_back(1);
        int prev = -1, cur = 1;
        for (int step = 1; step < n; step++) {
            int nxt = -1;
            for (int v : adj[cur]) if (v != prev) { nxt = v; break; }
            if (nxt == -1) break;
            perm.push_back(nxt);
            prev = cur;
            cur = nxt;
        }

        cout << -1;
        for (int x : perm) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    } else {
        // Regime "chain-walk-quad" new direction: K=sqrt(n) SEED skeleton.
        // Replaces per-step linear scan with persistent-S amortization.
        // Phase A: ONE query installs K seeds in S then probes each non-seed
        //          [v,v]. bit_after_add reveals "v adj some seed".
        // Phase B: For each adj-non-seed (~2K of them), binary search seeds
        //          by toggling subset out of S to find which seed it is adj to.
        // Phase C placeholder: identity guess; parallel chain walk pending.
        int K = (int)max(2.0, ceil(sqrt((double)n)));
        if (K > n / 4) K = n / 4;

        int stp = n / K;
        if (stp < 2) stp = 2;
        vector<int> seeds;
        for (int i = 0; i < K; i++) {
            int s = 1 + i * stp;
            if (s > n) break;
            seeds.push_back(s);
        }
        K = (int)seeds.size();

        vector<char> isSeed(n + 1, 0);
        for (int s : seeds) isSeed[s] = 1;

        vector<int> nonSeeds;
        nonSeeds.reserve(n - K);
        for (int v = 1; v <= n; v++) if (!isSeed[v]) nonSeeds.push_back(v);

        long long L1 = (long long)K + 2LL * (long long)nonSeeds.size();
        cout << L1;
        for (int s : seeds) cout << ' ' << s;
        for (int v : nonSeeds) cout << ' ' << v << ' ' << v;
        cout << '\n';
        cout.flush();

        vector<int> r1(L1);
        for (long long k = 0; k < L1; k++) cin >> r1[k];

        int bitAfterSeeds = r1[K - 1];
        if (bitAfterSeeds != 0) {
            vector<int> perm(n);
            for (int i = 0; i < n; i++) perm[i] = i + 1;
            outputGuess(perm);
            return 0;
        }

        vector<int> adjToSeeds;
        for (size_t i = 0; i < nonSeeds.size(); i++) {
            long long add_idx = (long long)K + 2LL * (long long)i;
            if (r1[add_idx] == 1) adjToSeeds.push_back(nonSeeds[i]);
        }

        if ((int)adjToSeeds.size() != 2 * K) {
            vector<int> perm(n);
            for (int i = 0; i < n; i++) perm[i] = i + 1;
            outputGuess(perm);
            return 0;
        }

        vector<int> seedIdxOfV(adjToSeeds.size(), -1);

        for (size_t idx = 0; idx < adjToSeeds.size(); idx++) {
            int v = adjToSeeds[idx];
            int lo = 0, hi = K - 1;
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                vector<int> ops;
                ops.reserve(2 * (mid - lo + 1) + 2);
                for (int j = lo; j <= mid; j++) ops.push_back(seeds[j]);
                ops.push_back(v);
                ops.push_back(v);
                for (int j = lo; j <= mid; j++) ops.push_back(seeds[j]);

                long long Lq = (long long)ops.size();
                cout << Lq;
                for (int x : ops) cout << ' ' << x;
                cout << '\n';
                cout.flush();

                vector<int> rq(Lq);
                for (long long k = 0; k < Lq; k++) cin >> rq[k];

                int removeCount = mid - lo + 1;
                int bitAfterVAdd = rq[removeCount];

                if (bitAfterVAdd == 1) {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            seedIdxOfV[idx] = lo;
        }

        vector<vector<int>> seedNbrs(K);
        for (size_t idx = 0; idx < adjToSeeds.size(); idx++) {
            seedNbrs[seedIdxOfV[idx]].push_back(adjToSeeds[idx]);
        }

        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i + 1;
        outputGuess(perm);
    }
    return 0;
}
