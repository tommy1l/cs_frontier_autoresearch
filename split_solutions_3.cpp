#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int subtask, n;
    cin >> subtask >> n;

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
        // Regime "indep-bsearch-skel": now with Phase A verify + Phase C binary-encoded bsearch.
        // K small enough that {1..K} likely indep on cycle (P~exp(-K^2/n)).
        // Phase A: toggle {1..K} in, verify all responses 0.
        // Phase B: probe each u in K+1..n with (u,u), identify N(S).
        // Phase C: for bit b in 0..ceil(log2(K))-1, set S to {i: bit b of (i-1) set},
        //          probe each u in N(S), record bit b of u's S-neighbor index.
        // Multi-round and cycle reconstruction not yet implemented -> emit identity.
        int K = 200;

        // Phase A
        cout << K;
        for (int i = 1; i <= K; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        vector<int> rA(K);
        for (int& r : rA) cin >> r;

        bool indep = true;
        for (int r : rA) if (r) { indep = false; break; }
        if (!indep) {
            cout << -1;
            for (int i = 1; i <= n; i++) cout << ' ' << i;
            cout << '\n';
            cout.flush();
            return 0;
        }

        // Phase B
        long long U = n - K;
        long long LB = 2LL * U;
        cout << LB;
        for (int u = K + 1; u <= n; u++) cout << ' ' << u << ' ' << u;
        cout << '\n';
        cout.flush();
        vector<int> rB(LB);
        for (int& r : rB) cin >> r;

        vector<int> NS;
        for (long long idx = 0; idx < U; idx++) {
            if (rB[2*idx] == 1) NS.push_back(K + 1 + (int)idx);
        }

        // Phase C: binary-encoded parallel binary search
        int B = 1;
        while ((1 << B) < K) B++;
        vector<int> cur_in(K + 1, 1);
        vector<int> nbr_idx(NS.size(), 0);

        for (int b = 0; b < B; b++) {
            vector<int> target(K + 1, 0);
            for (int i = 1; i <= K; i++) {
                if (((i - 1) >> b) & 1) target[i] = 1;
            }
            vector<int> ops;
            for (int i = 1; i <= K; i++) {
                if (cur_in[i] != target[i]) {
                    ops.push_back(i);
                    cur_in[i] = target[i];
                }
            }
            int probe_start = (int)ops.size();
            for (int u : NS) { ops.push_back(u); ops.push_back(u); }

            long long L = ops.size();
            cout << L;
            for (int op : ops) cout << ' ' << op;
            cout << '\n';
            cout.flush();
            vector<int> r(L);
            for (int& x : r) cin >> x;

            for (size_t i = 0; i < NS.size(); i++) {
                int first_r = r[probe_start + 2*(int)i];
                if (first_r == 1) nbr_idx[i] |= (1 << b);
            }
        }

        (void)nbr_idx;

        // Emit identity (multi-round + reconstruction not implemented).
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
