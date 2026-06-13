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
        // NEW REGIME "multi-anchor-batch":
        // Honors DIRECTIVES.TRY "amortize many chain decisions into one query"
        // and "exploit persistent-S so toggle cost is paid once and reused".
        // Phase 0: ONE big query of ~K*2(n-1) ops identifies the 2 ring-neighbors
        // of EACH anchor in {1..K=49}. Persistent-S {anchor} switched K-1 times
        // (2 ops each switch), probing every other vertex with (v,v) per anchor.
        // Phase 1: chain walk from anchor 1's segment, using anchor edges as
        // shortcuts (free hops at anchors) plus budget-capped per-step probes.

        const int K = 49;
        const long long totalOpCap = 14000000LL;
        long long opsUsed = 0;

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
        opsUsed += L0;

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

        vector<int> chain;
        vector<char> placed(n + 1, 0);
        int curSetVertex = K;

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
                    long long opsNeeded = (curSetVertex != cur ? 2LL : 0LL) + 2LL * remaining;
                    if (opsUsed + opsNeeded > totalOpCap) break;

                    vector<int> q;
                    q.reserve(opsNeeded);
                    if (curSetVertex != cur) {
                        q.push_back(curSetVertex);
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

                    long long pidx = (curSetVertex != cur) ? 2LL : 0LL;
                    for (int v = 1; v <= n; v++) {
                        if (placed[v] || v == cur) continue;
                        if (rq[pidx] == 1) {
                            bool already = false;
                            for (int x : nbrs[cur]) if (x == v) { already = true; break; }
                            if (!already) {
                                nbrs[cur].push_back(v);
                                nbrs[v].push_back(cur);
                            }
                        }
                        pidx += 2;
                    }
                    curSetVertex = cur;

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
