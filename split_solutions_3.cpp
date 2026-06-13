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
        // naive-bsearch-walk: pollution-free brute neighbor test per chain step.
        // Per step: 1 query, S={cur}, for each unplaced u: [u, u].
        // bit after [u in] = adj(cur, u); bit after [u out] = 0.
        // No pollution: |S| <= 2, never two unplaced together.
        // Cost: per step ~2m ops; total ~n^2 ops -> budget cap hits mid-walk
        // -> identity-fill remainder -> subtask 2 still wrong. Isolates
        // pollution as a variable; budget still the limiting wall.

        long long opCap = 280000000LL;
        long long opsUsed = 0;

        // Phase 1: find N(1). Query: [1, 2,2, 3,3, ..., n,n]. After: S={1}.
        long long L0 = 1LL + 2LL * (n - 1);
        cout << L0;
        cout << ' ' << 1;
        for (int v = 2; v <= n; v++) cout << ' ' << v << ' ' << v;
        cout << '\n';
        cout.flush();
        opsUsed += L0;

        vector<int> r0(L0);
        for (long long k = 0; k < L0; k++) cin >> r0[k];

        int nbA = -1, nbB = -1;
        for (int k = 0; k < n - 1; k++) {
            int v = k + 2;
            int bitOn = r0[1 + 2 * k];
            if (bitOn == 1) {
                if (nbA == -1) nbA = v;
                else if (nbB == -1) nbB = v;
            }
        }

        vector<int> chain;
        vector<char> placed(n + 1, 0);

        if (nbA == -1 || nbB == -1) {
            for (int v = 1; v <= n; v++) chain.push_back(v);
            cout << -1;
            for (int x : chain) cout << ' ' << x;
            cout << '\n';
            cout.flush();
            return 0;
        }

        chain.push_back(nbA);
        chain.push_back(1);
        chain.push_back(nbB);
        placed[nbA] = placed[1] = placed[nbB] = 1;

        int prev = 1, cur = nbB;
        int curS = 1; // S = {1} after Phase 1

        bool exhausted = false;
        while ((int)chain.size() < n) {
            vector<int> U;
            U.reserve(n);
            for (int v = 1; v <= n; v++) if (!placed[v]) U.push_back(v);
            int m = (int)U.size();
            if (m == 0) break;
            if (m == 1) {
                chain.push_back(U[0]);
                placed[U[0]] = 1;
                break;
            }

            vector<int> ops;
            ops.reserve(2 + 2LL * m);
            int switchOps = 0;
            if (curS != cur) {
                ops.push_back(curS);
                ops.push_back(cur);
                switchOps = 2;
            }
            for (int u : U) {
                ops.push_back(u);
                ops.push_back(u);
            }

            long long Lq = ops.size();
            if (opsUsed + Lq > opCap) {
                exhausted = true;
                break;
            }

            cout << Lq;
            for (int x : ops) cout << ' ' << x;
            cout << '\n';
            cout.flush();
            opsUsed += Lq;

            vector<int> r(Lq);
            for (long long k = 0; k < Lq; k++) cin >> r[k];

            curS = cur;

            int nxt = -1;
            for (int j = 0; j < m; j++) {
                int bitIn = r[switchOps + 2 * j];
                if (bitIn == 1) {
                    nxt = U[j];
                    break;
                }
            }

            if (nxt == -1) {
                exhausted = true;
                break;
            }

            chain.push_back(nxt);
            placed[nxt] = 1;
            prev = cur;
            cur = nxt;
        }

        if (exhausted) {
            for (int v = 1; v <= n; v++) {
                if (!placed[v]) chain.push_back(v);
            }
        }

        cout << -1;
        for (int x : chain) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
