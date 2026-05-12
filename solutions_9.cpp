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

        vector<bitset<MAXN>> inSubtree(n + 1);
        for (int i = 1; i <= n; i++) inSubtree[i].set(i);
        for (int i = (int)order_arr.size() - 1; i >= 0; i--) {
            int u = order_arr[i];
            if (par[u] != 0) inSubtree[par[u]] |= inSubtree[u];
        }

        // variant bits:
        //   bit 0: 0 = deepest-first greedy, 1 = shallowest-first greedy
        //   bit 1: 0 = Pass 2 only when Pass 1 empty, 1 = always Pass 2
        auto runStrategy = [&](int variant) -> vector<vector<int>> {
            vector<int> q = p_init;
            vector<vector<int>> ops_local;
            int maxOps = 6 * n + 200;
            auto isSortedQ = [&]() {
                for (int i = 1; i <= n; i++) if (q[i] != i) return false;
                return true;
            };
            int sz = (int)order_arr.size();
            while ((int)ops_local.size() < maxOps && !isSortedQ()) {
                vector<int> matched(n + 1, 0);
                vector<int> chosen;

                for (int idx = 0; idx < sz; idx++) {
                    int i = (variant & 1) ? idx : sz - 1 - idx;
                    int v = order_arr[i];
                    if (par[v] == 0) continue;
                    int u = par[v];
                    if (matched[v] || matched[u]) continue;
                    bool gv = !inSubtree[v][q[v]];
                    bool gu = inSubtree[v][q[u]];
                    if (gv && gu) {
                        matched[v] = matched[u] = 1;
                        chosen.push_back(edgeToParent[v]);
                    }
                }

                bool runPass2 = (variant & 2) || chosen.empty();
                if (runPass2) {
                    for (int idx = 0; idx < sz; idx++) {
                        int i = (variant & 1) ? idx : sz - 1 - idx;
                        int v = order_arr[i];
                        if (par[v] == 0) continue;
                        int u = par[v];
                        if (matched[v] || matched[u]) continue;
                        bool gv = !inSubtree[v][q[v]];
                        bool gu = inSubtree[v][q[u]];
                        if (gv || gu) {
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
            if (!isSortedQ()) return {{-1}}; // signal failure
            return ops_local;
        };

        vector<vector<int>> best;
        bool haveBest = false;
        for (int variant = 0; variant < 4; variant++) {
            auto res = runStrategy(variant);
            if (!res.empty() && res[0].size() == 1 && res[0][0] == -1) continue;
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
