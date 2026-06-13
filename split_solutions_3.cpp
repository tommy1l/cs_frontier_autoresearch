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
        // Regime "naive-bsearch-walk", non-reverting persistent-prefix variant.
        // Anchor (8cedb00) submitted [in...,out...] per bsearch level => ~2m ops/step.
        // Here each level submits only the symmetric-diff toggle to move
        // S = {cur} ∪ U[lit_lo..lit_hi) to the new test range U[lo..mid).
        // Per-step worst case still O(m) (all-right path requires full revert),
        // but expected closer to m vs 2m. Pollution behaviour unchanged
        // (U[lo..mid) internal adj still flips top-level decisions).
        // Goal: test whether shaving the toggle-out half changes the score wall.

        long long opCap = 250000000LL;
        long long opsUsed = 0;

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

        int cur = nbB;
        int curS = 1; // after round 0 toggles, S = {1}

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

            // Reset S to {cur}.
            if (curS != cur) {
                vector<int> tOps = {curS, cur};
                long long Lt = (long long)tOps.size();
                if (opsUsed + Lt > opCap) break;
                cout << Lt;
                for (int x : tOps) cout << ' ' << x;
                cout << '\n';
                cout.flush();
                vector<int> rt(Lt);
                for (long long k = 0; k < Lt; k++) cin >> rt[k];
                opsUsed += Lt;
                curS = cur;
            }

            // Non-reverting bsearch.
            // Invariant: S = {cur} ∪ U[lit_lo..lit_hi).
            // Test by ensuring S has U[lo..mid). At end of each query, lit_lo/lit_hi = lo/mid.
            int lo = 0, hi = m;
            int lit_lo = 0, lit_hi = 0;
            bool aborted = false;

            while (hi - lo > 1) {
                int mid = (lo + hi) / 2;
                // Symmetric difference of [lit_lo, lit_hi) and [lo, mid) as toggle ops.
                vector<int> ops;
                int aL = lit_lo, aR = lit_hi, bL = lo, bR = mid;
                if (aR <= bL || bR <= aL) {
                    ops.reserve((aR - aL) + (bR - bL));
                    for (int k = aL; k < aR; k++) ops.push_back(U[k]);
                    for (int k = bL; k < bR; k++) ops.push_back(U[k]);
                } else {
                    int L_low = min(aL, bL), L_high = max(aL, bL);
                    int R_low = min(aR, bR), R_high = max(aR, bR);
                    ops.reserve((L_high - L_low) + (R_high - R_low));
                    for (int k = L_low; k < L_high; k++) ops.push_back(U[k]);
                    for (int k = R_low; k < R_high; k++) ops.push_back(U[k]);
                }

                if (ops.empty()) {
                    // No diff; shouldn't happen for bsearch with mid != prev test ranges.
                    break;
                }

                long long Lq = (long long)ops.size();
                if (opsUsed + Lq > opCap) { aborted = true; break; }
                cout << Lq;
                for (int x : ops) cout << ' ' << x;
                cout << '\n';
                cout.flush();
                vector<int> r(Lq);
                for (long long k = 0; k < Lq; k++) cin >> r[k];
                opsUsed += Lq;

                int bitTest = r[Lq - 1];
                lit_lo = lo;
                lit_hi = mid;

                if (bitTest == 1) hi = mid;
                else lo = mid;
            }

            if (aborted) break;

            int nxt_idx = lo;
            if (nxt_idx < 0 || nxt_idx >= m) break;
            int nxt = U[nxt_idx];
            chain.push_back(nxt);
            placed[nxt] = 1;

            // Reset S = {nxt}: turn cur off, turn every lit U-element off (keep nxt if already lit, else turn on).
            vector<int> resetOps;
            resetOps.push_back(cur);
            bool nxt_was_lit = (nxt_idx >= lit_lo && nxt_idx < lit_hi);
            for (int k = lit_lo; k < lit_hi; k++) {
                if (k == nxt_idx) continue;
                resetOps.push_back(U[k]);
            }
            if (!nxt_was_lit) resetOps.push_back(nxt);

            long long Lr = (long long)resetOps.size();
            if (Lr > 0) {
                if (opsUsed + Lr > opCap) { aborted = true; break; }
                cout << Lr;
                for (int x : resetOps) cout << ' ' << x;
                cout << '\n';
                cout.flush();
                vector<int> rr(Lr);
                for (long long k = 0; k < Lr; k++) cin >> rr[k];
                opsUsed += Lr;
            }

            cur = nxt;
            curS = nxt;
            if (aborted) break;
        }

        for (int v = 1; v <= n; v++) if (!placed[v]) chain.push_back(v);

        cout << -1;
        for (int x : chain) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
