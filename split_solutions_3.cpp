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
        // REGIME "multi-anchor-batch", TRIAL: layered batched stitch.
        // Phase 0 (unchanged, K=49): one big query identifying ring-neighbors
        //   of every anchor a in {1..K}.
        // Phase 1 (NEW): one big query that, for every anchor-neighbor w,
        //   identifies w's OTHER ring-neighbor via persistent S={w} + (v,v) probes.
        //   Honors TRY(1) "stitch K anchor segments via BATCH queries" and
        //   TRY(2) "spend more on phase-0-style batched queries between segments".
        // Walk: known-edges only, NO per-step (v,v) fallback (AVOID compliance).

        const int K = 49;

        vector<char> is_anchor(n + 1, 0);
        for (int a = 1; a <= K; a++) is_anchor[a] = 1;

        // ---- PHASE 0 ----
        vector<int> ops;
        long long phase0Ops = 1LL + 2LL * (n - 1) + (long long)(K - 1) * (2LL + 2LL * (n - 1));
        ops.reserve(phase0Ops);

        ops.push_back(1);
        for (int v = 2; v <= n; v++) {
            ops.push_back(v);
            ops.push_back(v);
        }
        for (int a = 2; a <= K; a++) {
            ops.push_back(a - 1);
            ops.push_back(a);
            for (int v = 1; v <= n; v++) {
                if (v == a) continue;
                ops.push_back(v);
                ops.push_back(v);
            }
        }

        long long L0 = ops.size();
        cout << L0;
        for (int x : ops) cout << ' ' << x;
        cout << '\n';
        cout.flush();

        vector<int> resp(L0);
        for (long long k = 0; k < L0; k++) cin >> resp[k];

        vector<vector<int>> nbrs(n + 1);
        auto addEdge = [&](int u, int w) {
            for (int x : nbrs[u]) if (x == w) return;
            nbrs[u].push_back(w);
            nbrs[w].push_back(u);
        };

        long long idx = 1;
        for (int v = 2; v <= n; v++) {
            if (resp[idx] == 1) addEdge(1, v);
            idx += 2;
        }
        for (int a = 2; a <= K; a++) {
            idx += 2;
            for (int v = 1; v <= n; v++) {
                if (v == a) continue;
                if (resp[idx] == 1) addEdge(a, v);
                idx += 2;
            }
        }
        // S now = {K} after phase 0

        // ---- PHASE 1: layered batched stitch ----
        // Collect distinct non-anchor anchor-neighbors W.
        vector<char> inW(n + 1, 0);
        vector<int> W;
        for (int a = 1; a <= K; a++) {
            for (int w : nbrs[a]) {
                if (w >= 1 && w <= n && !is_anchor[w] && !inW[w]) {
                    inW[w] = 1;
                    W.push_back(w);
                }
            }
        }

        if (!W.empty()) {
            vector<int> ops1;
            ops1.reserve((long long)W.size() * (2 + 2 * (n - 1)));
            int curFront = K;
            vector<long long> probeStarts;
            probeStarts.reserve(W.size());

            for (int w : W) {
                ops1.push_back(curFront);
                ops1.push_back(w);
                probeStarts.push_back((long long)ops1.size());
                for (int v = 1; v <= n; v++) {
                    if (v == w) continue;
                    ops1.push_back(v);
                    ops1.push_back(v);
                }
                curFront = w;
            }

            long long L1 = ops1.size();
            cout << L1;
            for (int x : ops1) cout << ' ' << x;
            cout << '\n';
            cout.flush();

            vector<int> resp1(L1);
            for (long long k = 0; k < L1; k++) cin >> resp1[k];

            for (size_t i = 0; i < W.size(); i++) {
                int w = W[i];
                long long base = probeStarts[i];
                long long off = 0;
                for (int v = 1; v <= n; v++) {
                    if (v == w) continue;
                    if (resp1[base + off] == 1) {
                        addEdge(w, v);
                    }
                    off += 2;
                }
            }
        }

        // ---- Walk: known edges only (no fallback) ----
        vector<int> chain;
        vector<char> placed(n + 1, 0);

        if ((int)nbrs[1].size() >= 2) {
            int a = nbrs[1][0], b = nbrs[1][1];
            chain.push_back(a); placed[a] = 1;
            chain.push_back(1); placed[1] = 1;
            chain.push_back(b); placed[b] = 1;

            // Forward walk from b
            while ((int)chain.size() < n) {
                int prev = chain[chain.size() - 2];
                int cur = chain.back();
                int nxt = -1;
                for (int v : nbrs[cur]) {
                    if (v != prev && !placed[v]) { nxt = v; break; }
                }
                if (nxt == -1) break;
                chain.push_back(nxt); placed[nxt] = 1;
            }

            // Backward walk from front (a side)
            deque<int> dq(chain.begin(), chain.end());
            while ((int)dq.size() < n) {
                int next2 = dq[1];
                int cur = dq.front();
                int nxt = -1;
                for (int v : nbrs[cur]) {
                    if (v != next2 && !placed[v]) { nxt = v; break; }
                }
                if (nxt == -1) break;
                dq.push_front(nxt); placed[nxt] = 1;
            }
            chain.assign(dq.begin(), dq.end());
        }

        // Fill remaining as identity (chain will be incomplete; AVOID partial-skeleton
        // is noted — this trial isolates whether layered-stitch alone closes any arcs).
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
