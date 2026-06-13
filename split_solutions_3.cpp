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
        // Regime "indep-bsearch-skel": close Phases A-B-C end-to-end.
        // Phase A: build S={1..K}, K=40. Verify indep (K^2/n=0.016 expected adj).
        // Phase B: 1 query (2(n-K) ops) tags v in V\S that are adj to some s in S.
        //          Bit after [v add] = (v adj S) since S is indep.
        // Phase C: 1 BIG query. For each tagged v, for each bit b in [0..BITS),
        //          toggle out Aout[b]={s: bit_b(s)=0}, probe [v,v], toggle back.
        //          Bit at "v add" = (v adj A_b). Decode 6 bits -> specific s in [1..K].
        //          ~252 ops per v; up to 2K=80 tagged v's -> ~20k ops.
        // Phase D: TODO. Output identity placeholder. Without ring reconstruction,
        //          n=1e5 still scores 0; but Phase A-B-C structure is now real.
        // Total t=3, Q ~ 2.2e5. If Phase D closed correctly, lambda -> ~1.

        const int K = 40;
        const int BITS = 6;

        cout << K;
        for (int i = 1; i <= K; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();

        vector<int> rA(K);
        for (auto& x : rA) cin >> x;

        bool indep = true;
        for (int i = 0; i < K; i++) if (rA[i] != 0) { indep = false; break; }

        if (!indep) {
            cout << -1;
            for (int i = 1; i <= n; i++) cout << ' ' << i;
            cout << '\n';
            cout.flush();
            return 0;
        }

        long long LB = 2LL * (n - K);
        cout << LB;
        for (int v = K + 1; v <= n; v++) cout << ' ' << v << ' ' << v;
        cout << '\n';
        cout.flush();

        vector<int> rB(LB);
        for (auto& x : rB) cin >> x;

        vector<int> adjV;
        for (int j = 0; j < n - K; j++) {
            int v = K + 1 + j;
            if (rB[2 * j] == 1) adjV.push_back(v);
        }

        vector<int> sForV(adjV.size(), -1);
        if (!adjV.empty()) {
            vector<vector<int>> Aout(BITS);
            for (int b = 0; b < BITS; b++) {
                for (int s = 1; s <= K; s++) {
                    if (((s >> b) & 1) == 0) Aout[b].push_back(s);
                }
            }

            vector<int> ops;
            long long opCount = 0;
            for (int b = 0; b < BITS; b++) opCount += (long long)adjV.size() * (2LL * Aout[b].size() + 2);
            ops.reserve(opCount);

            for (int v : adjV) {
                for (int b = 0; b < BITS; b++) {
                    for (int s : Aout[b]) ops.push_back(s);
                    ops.push_back(v);
                    ops.push_back(v);
                    for (int s : Aout[b]) ops.push_back(s);
                }
            }

            long long LC = ops.size();
            cout << LC;
            for (int x : ops) cout << ' ' << x;
            cout << '\n';
            cout.flush();

            vector<int> rC(LC);
            for (auto& x : rC) cin >> x;

            long long idx = 0;
            for (size_t vi = 0; vi < adjV.size(); vi++) {
                int sBits = 0;
                for (int b = 0; b < BITS; b++) {
                    int outSize = (int)Aout[b].size();
                    idx += outSize;
                    int bitAfterVAdd = rC[idx];
                    idx++;
                    idx++;
                    idx += outSize;
                    if (bitAfterVAdd == 1) sBits |= (1 << b);
                }
                if (sBits >= 1 && sBits <= K) sForV[vi] = sBits;
            }
        }
        (void)sForV;

        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
