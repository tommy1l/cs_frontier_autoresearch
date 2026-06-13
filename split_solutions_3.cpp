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
        // Regime "chain-walk-quad": close end-to-end with chain reconstruction.
        // Round 0: S={}->{1}; [v,v] scan v=2..n with S={1}. Find 1's nbrs A,B.
        // Round k>=1: switch S from {prev} to {cur}, then [v,v] scan unplaced.
        //   Unique v with bit=1 is cur's other neighbor (not prev).
        // Total ops ~ n^2; this trial closes the regime, exposing the budget
        // ceiling. Future trials must replace per-step linear scan.

        vector<char> placed(n + 1, 0);
        vector<int> chain;
        chain.reserve(n);

        long long L0 = 1LL + 2LL * (n - 1);
        cout << L0;
        cout << ' ' << 1;
        for (int v = 2; v <= n; v++) cout << ' ' << v << ' ' << v;
        cout << '\n';
        cout.flush();

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

        if (nbA != -1 && nbB != -1) {
            chain.push_back(nbA);
            chain.push_back(1);
            chain.push_back(nbB);
            placed[nbA] = placed[1] = placed[nbB] = 1;

            int prev = 1, cur = nbB;

            while ((int)chain.size() < n) {
                int remaining = n - (int)chain.size();
                vector<int> ops;
                ops.reserve(2 + 2 * remaining);
                ops.push_back(prev);
                ops.push_back(cur);
                for (int v = 1; v <= n; v++) {
                    if (placed[v]) continue;
                    ops.push_back(v);
                    ops.push_back(v);
                }
                long long Lq = ops.size();
                cout << Lq;
                for (int x : ops) cout << ' ' << x;
                cout << '\n';
                cout.flush();

                vector<int> rq(Lq);
                for (long long k = 0; k < Lq; k++) cin >> rq[k];

                int nxt = -1;
                int idx = 2;
                for (int v = 1; v <= n; v++) {
                    if (placed[v]) continue;
                    int bitOn = rq[idx];
                    if (bitOn == 1) {
                        nxt = v;
                        break;
                    }
                    idx += 2;
                }

                if (nxt == -1) break;
                chain.push_back(nxt);
                placed[nxt] = 1;
                prev = cur;
                cur = nxt;
            }
        }

        if ((int)chain.size() != n) {
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
