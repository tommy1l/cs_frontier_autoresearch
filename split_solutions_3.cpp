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
        // Regime "indep-bsearch-skel": foundation for sub-quadratic n=1e5.
        // Phase A: assume S={1..K} indep (P>0.97 for K=50 since K^2/n=0.025).
        // Phase B: probe each u in V\S for adjacency to S in one batched query.
        // Future trials: add parallel bsearch to identify which s, recurse on V\S.
        int K = 50;

        cout << K;
        for (int i = 1; i <= K; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();

        vector<int> rA(K);
        for (auto& x : rA) cin >> x;

        int U = n - K;
        long long L = 2LL * U;
        cout << L;
        for (int u = K + 1; u <= n; u++) cout << ' ' << u << ' ' << u;
        cout << '\n';
        cout.flush();

        vector<int> rB(L);
        for (auto& x : rB) cin >> x;
        (void)rA; (void)rB;

        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
