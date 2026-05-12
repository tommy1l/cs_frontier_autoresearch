#include <bits/stdc++.h>
using namespace std;

const int MAXN = 1005;

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        int n;
        scanf("%d", &n);
        vector<int> p(n + 1);
        for (int i = 1; i <= n; i++) scanf("%d", &p[i]);
        vector<vector<int>> adj(n + 1);
        vector<pair<int, int>> edges(n + 1);
        for (int i = 1; i <= n - 1; i++) {
            int u, v;
            scanf("%d %d", &u, &v);
            adj[u].push_back(v);
            adj[v].push_back(u);
            edges[i] = {u, v};
        }

        // Root tree at 1, compute BFS order, parents, edgeToParent
        vector<int> par(n + 1, 0);
        vector<int> order_arr;
        vector<int> edgeToParent(n + 1, 0);
        vector<bool> visited(n + 1, false);
        {
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

        // inSubtree[v] = bitset of vertices in subtree rooted at v
        vector<bitset<MAXN>> inSubtree(n + 1);
        for (int i = 1; i <= n; i++) inSubtree[i].set(i);
        for (int i = (int)order_arr.size() - 1; i >= 0; i--) {
            int u = order_arr[i];
            if (par[u] != 0) inSubtree[par[u]] |= inSubtree[u];
        }

        vector<vector<int>> ops;
        auto isSorted = [&]() {
            for (int i = 1; i <= n; i++) if (p[i] != i) return false;
            return true;
        };

        // Edge (u=par[v], v): good for v iff p[v] is NOT in subtree(v)
        //                     good for u iff p[u] IS in subtree(v)
        // Phase 1: pick max matching from double-good edges (greedy deepest-first).
        // Phase 2 (only when phase 1 empty): pick matching from single-good edges to break stalls.

        int maxOps = 4 * n + 50;
        while ((int)ops.size() < maxOps && !isSorted()) {
            vector<int> matched(n + 1, 0);
            vector<int> chosen;

            for (int i = (int)order_arr.size() - 1; i >= 0; i--) {
                int v = order_arr[i];
                if (par[v] == 0) continue;
                int u = par[v];
                if (matched[v] || matched[u]) continue;
                bool gv = !inSubtree[v][p[v]];
                bool gu = inSubtree[v][p[u]];
                if (gv && gu) {
                    matched[v] = matched[u] = 1;
                    chosen.push_back(edgeToParent[v]);
                }
            }

            if (chosen.empty()) {
                for (int i = (int)order_arr.size() - 1; i >= 0; i--) {
                    int v = order_arr[i];
                    if (par[v] == 0) continue;
                    int u = par[v];
                    if (matched[v] || matched[u]) continue;
                    bool gv = !inSubtree[v][p[v]];
                    bool gu = inSubtree[v][p[u]];
                    if (gv || gu) {
                        matched[v] = matched[u] = 1;
                        chosen.push_back(edgeToParent[v]);
                    }
                }
            }

            if (chosen.empty()) break;

            for (int eid : chosen) {
                int u = edges[eid].first, v = edges[eid].second;
                swap(p[u], p[v]);
            }
            ops.push_back(chosen);
        }

        printf("%d\n", (int)ops.size());
        for (auto& op : ops) {
            printf("%d", (int)op.size());
            for (int e : op) printf(" %d", e);
            printf("\n");
        }
    }
    return 0;
}
