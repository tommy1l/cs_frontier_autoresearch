#include <bits/stdc++.h>
using namespace std;

const int MAXN = 1005;

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        int n;
        scanf("%d", &n);
        vector<int> p_init(n + 1);
        for (int i = 1; i <= n; i++) scanf("%d", &p_init[i]);
        vector<vector<int>> adj(n + 1);
        vector<pair<int, int>> edges(n + 1);
        for (int i = 1; i <= n - 1; i++) {
            int u, v;
            scanf("%d %d", &u, &v);
            adj[u].push_back(v);
            adj[v].push_back(u);
            edges[i] = {u, v};
        }

        vector<int> par(n + 1, 0);
        vector<int> order_arr;
        vector<int> edgeToParent(n + 1, 0);
        {
            vector<bool> visited(n + 1, false);
            queue<int> qq;
            qq.push(1);
            visited[1] = true;
            while (!qq.empty()) {
                int u = qq.front(); qq.pop();
                order_arr.push_back(u);
                for (int v : adj[u]) {
                    if (!visited[v]) {
                        visited[v] = true;
                        par[v] = u;
                        qq.push(v);
                    }
                }
            }
        }
        for (int i = 1; i <= n - 1; i++) {
            int u = edges[i].first, v = edges[i].second;
            if (par[u] == v) edgeToParent[u] = i;
            else if (par[v] == u) edgeToParent[v] = i;
        }

        vector<bitset<MAXN>> inSubtree(n + 1);
        for (int i = 1; i <= n; i++) inSubtree[i].set(i);
        for (int i = (int)order_arr.size() - 1; i >= 0; i--) {
            int u = order_arr[i];
            if (par[u] != 0) inSubtree[par[u]] |= inSubtree[u];
        }

        // Helpers
        int sz = (int)order_arr.size();

        // For each vertex v, compute (gv, gu) wrt edge (v, par[v]) in state q.
        // gv=true iff p[v] not in subtree(v); gu=true iff p[u]=p[par[v]] in subtree(v).
        // Returns 0 = neither, 1 = single-good for v only, 2 = single-good for u only,
        //         3 = double-good.
        auto edgeStatus = [&](int v, const vector<int>& q) -> int {
            if (par[v] == 0) return 0;
            int u = par[v];
            bool gv = !inSubtree[v][q[v]];
            bool gu = inSubtree[v][q[u]];
            return (gv ? 1 : 0) | (gu ? 2 : 0);
        };

        // Check if any edge in tree is double-good in given state.
        auto hasDoubleGood = [&](const vector<int>& q) -> bool {
            for (int v = 1; v <= n; v++) {
                if (par[v] == 0) continue;
                if (edgeStatus(v, q) == 3) return true;
            }
            return false;
        };

        // variant encoding:
        //   bit 0: greedy order (0=deepest first, 1=shallowest first)
        //   bits 1-2: Pass 2 mode
        //     0: only when Pass 1 empty
        //     1: always (combined)
        //     2: lookahead - only single-good edges that produce next-round double-good
        //     3: only when Pass 1 empty + lookahead
        auto runStrategy = [&](int variant) -> vector<vector<int>> {
            vector<int> q = p_init;
            vector<vector<int>> ops_local;
            int maxOps = 8 * n + 500;
            auto isSortedQ = [&]() {
                for (int i = 1; i <= n; i++) if (q[i] != i) return false;
                return true;
            };
            int shallowFirst = variant & 1;
            int p2mode = (variant >> 1) & 3;
            while ((int)ops_local.size() < maxOps && !isSortedQ()) {
                vector<int> matched(n + 1, 0);
                vector<int> chosen;

                // Pass 1: max-cardinality matching of double-good edges
                for (int idx = 0; idx < sz; idx++) {
                    int i = shallowFirst ? idx : sz - 1 - idx;
                    int v = order_arr[i];
                    if (par[v] == 0) continue;
                    int u = par[v];
                    if (matched[v] || matched[u]) continue;
                    if (edgeStatus(v, q) == 3) {
                        matched[v] = matched[u] = 1;
                        chosen.push_back(edgeToParent[v]);
                    }
                }

                bool needPass2 = false;
                if (p2mode == 0 || p2mode == 3) needPass2 = chosen.empty();
                else if (p2mode == 1 || p2mode == 2) needPass2 = true;

                if (needPass2) {
                    bool useLookahead = (p2mode == 2 || p2mode == 3);
                    // For lookahead, compute set of "useful" single-good edges:
                    // edges that, when applied alone, produce a double-good somewhere.
                    vector<bool> useful;
                    if (useLookahead) {
                        useful.assign(n + 1, false);
                        for (int v = 1; v <= n; v++) {
                            if (par[v] == 0) continue;
                            int u = par[v];
                            int s = edgeStatus(v, q);
                            if (s == 1 || s == 2) {
                                // Try applying
                                swap(q[u], q[v]);
                                if (hasDoubleGood(q)) useful[v] = true;
                                swap(q[u], q[v]);
                            }
                        }
                    }
                    for (int idx = 0; idx < sz; idx++) {
                        int i = shallowFirst ? idx : sz - 1 - idx;
                        int v = order_arr[i];
                        if (par[v] == 0) continue;
                        int u = par[v];
                        if (matched[v] || matched[u]) continue;
                        int s = edgeStatus(v, q);
                        if (s == 0 || s == 3) continue;
                        if (useLookahead && !useful[v]) continue;
                        matched[v] = matched[u] = 1;
                        chosen.push_back(edgeToParent[v]);
                    }
                    // If lookahead found nothing, fallback to any single-good
                    if (useLookahead && chosen.empty()) {
                        for (int idx = 0; idx < sz; idx++) {
                            int i = shallowFirst ? idx : sz - 1 - idx;
                            int v = order_arr[i];
                            if (par[v] == 0) continue;
                            int u = par[v];
                            if (matched[v] || matched[u]) continue;
                            int s = edgeStatus(v, q);
                            if (s == 0 || s == 3) continue;
                            matched[v] = matched[u] = 1;
                            chosen.push_back(edgeToParent[v]);
                        }
                    }
                }

                if (chosen.empty()) break;

                for (int eid : chosen) {
                    int u = edges[eid].first, v = edges[eid].second;
                    swap(q[u], q[v]);
                }
                ops_local.push_back(chosen);
            }
            if (!isSortedQ()) return {{-1}};
            return ops_local;
        };

        vector<vector<int>> best;
        bool haveBest = false;
        // Try variants: combinations of order (2) and p2mode (4) = 8 variants
        for (int variant = 0; variant < 8; variant++) {
            auto res = runStrategy(variant);
            bool fail = !res.empty() && res[0].size() == 1 && res[0][0] == -1;
            if (fail) continue;
            if (!haveBest || res.size() < best.size()) {
                best = res;
                haveBest = true;
            }
        }

        printf("%d\n", (int)best.size());
        for (auto& op : best) {
            printf("%d", (int)op.size());
            for (int e : op) printf(" %d", e);
            printf("\n");
        }
    }
    return 0;
}
