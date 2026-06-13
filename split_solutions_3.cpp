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
        // NEW REGIME "bit-fingerprint":
        // Replace the K*n Phase 0 probe with a log2(K)-query bit-indexed scheme.
        // For each bit b in 0..BITS-1, set S = {anchors with bit b set in their index}.
        // |S| ~ K/2 = 24 random anchors on the ring; expected #adj-pairs in S ~ 5e-3,
        // so A(S) = 0 with prob ~0.994. Probe each non-anchor v with (v,v):
        // response = "v adj to some anchor in S" = bit b of (OR of v's anchor-nbr indices).
        // After BITS queries, sigVals[v] = label-OR of v's anchor neighbors (0 if none).
        // Cost ~6 * 2n = 1.2e6 ops (vs 1e7 for old Phase 0). Then chain walk with
        // budget-capped per-step probe fallback.

        const int K = 128;
        const int BITS = 7;
        const long long opCap = 14000000LL;
        long long opsUsed = 0;

        vector<vector<int>> nbrs(n + 1);
        vector<char> inS(n + 1, 0);
        vector<int> sigVals(n + 1, 0);

        for (int b = 0; b < BITS; b++) {
            vector<int> ops;
            int transCount = 0;
            for (int a = 1; a <= K; a++) {
                int want = (a >> b) & 1;
                if (want != inS[a]) {
                    ops.push_back(a);
                    transCount++;
                }
            }
            for (int v = K + 1; v <= n; v++) {
                ops.push_back(v);
                ops.push_back(v);
            }

            long long L = ops.size();
            cout << L;
            for (int x : ops) cout << ' ' << x;
            cout << '\n';
            cout.flush();
            opsUsed += L;

            vector<int> resp(L);
            for (long long k = 0; k < L; k++) cin >> resp[k];

            for (int x : ops) inS[x] ^= 1;

            for (int v = K + 1; v <= n; v++) {
                long long idx = (long long)transCount + 2LL * (v - K - 1);
                if (resp[idx] == 1) sigVals[v] |= (1 << b);
            }
        }

        // Restore S to empty before chain walk
        {
            vector<int> ops;
            for (int a = 1; a <= K; a++) if (inS[a]) ops.push_back(a);
            if (!ops.empty()) {
                long long L = ops.size();
                cout << L;
                for (int x : ops) cout << ' ' << x;
                cout << '\n';
                cout.flush();
                opsUsed += L;
                vector<int> resp(L);
                for (long long k = 0; k < L; k++) cin >> resp[k];
                for (int x : ops) inS[x] ^= 1;
            }
        }

        // Build nbrs from signatures. sigVals[v] in [1..K] means v adj to single anchor s.
        for (int v = K + 1; v <= n; v++) {
            int s = sigVals[v];
            if (s >= 1 && s <= K) {
                nbrs[s].push_back(v);
                nbrs[v].push_back(s);
            }
        }

        vector<int> chain;
        vector<char> placed(n + 1, 0);
        int curSetVertex = 0;

        if (nbrs[1].size() >= 2) {
            int a = nbrs[1][0], b = nbrs[1][1];
            chain.push_back(a); placed[a] = 1;
            chain.push_back(1); placed[1] = 1;
            chain.push_back(b); placed[b] = 1;

            while ((int)chain.size() < n) {
                int prev = chain[chain.size() - 2];
                int cur = chain.back();
                int nxt = -1;

                for (int v : nbrs[cur]) {
                    if (v != prev && !placed[v]) { nxt = v; break; }
                }

                if (nxt == -1) {
                    int remaining = n - (int)chain.size();
                    long long switchOps = (curSetVertex == cur) ? 0LL : ((curSetVertex == 0) ? 1LL : 2LL);
                    long long opsNeeded = switchOps + 2LL * remaining;
                    if (opsUsed + opsNeeded > opCap) break;

                    vector<int> q;
                    q.reserve(opsNeeded);
                    if (curSetVertex != cur) {
                        if (curSetVertex != 0) q.push_back(curSetVertex);
                        q.push_back(cur);
                    }
                    for (int v = 1; v <= n; v++) {
                        if (placed[v] || v == cur) continue;
                        q.push_back(v);
                        q.push_back(v);
                    }
                    long long Lqq = q.size();
                    cout << Lqq;
                    for (int x : q) cout << ' ' << x;
                    cout << '\n';
                    cout.flush();
                    opsUsed += Lqq;

                    vector<int> rq(Lqq);
                    for (long long k = 0; k < Lqq; k++) cin >> rq[k];

                    long long pidx = switchOps;
                    curSetVertex = cur;

                    for (int v = 1; v <= n; v++) {
                        if (placed[v] || v == cur) continue;
                        if (rq[pidx] == 1) {
                            nbrs[cur].push_back(v);
                            nbrs[v].push_back(cur);
                        }
                        pidx += 2;
                    }

                    for (int v : nbrs[cur]) {
                        if (v != prev && !placed[v]) { nxt = v; break; }
                    }
                    if (nxt == -1) break;
                }

                chain.push_back(nxt);
                placed[nxt] = 1;
            }
        }

        for (int v = 1; v <= n; v++) {
            if (!placed[v]) chain.push_back(v);
        }

        cout << -1;
        for (int x : chain) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
