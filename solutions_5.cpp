/**
 * Problem 5: longest simple path in a directed graph (Hamiltonian guaranteed).
 *
 * Warnsdorff-style heuristic: at each step pick the unvisited neighbor with
 * the fewest unvisited out-edges. Start from vertices in ascending in-degree
 * order so true sources are tried first.
 */
#include <bits/stdc++.h>
using namespace std;

int n, m;
vector<vector<int>> adj, radj;
vector<int> in_deg;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    vector<int> a(10);
    for (int i = 0; i < 10; i++) cin >> a[i];
    adj.assign(n + 1, {});
    radj.assign(n + 1, {});
    in_deg.assign(n + 1, 0);
    for (int i = 0; i < m; i++) {
        int u, v; cin >> u >> v;
        adj[u].push_back(v);
        radj[v].push_back(u);
        in_deg[v]++;
    }

    auto start_time = chrono::steady_clock::now();
    auto time_left_ms = [&]() {
        auto now = chrono::steady_clock::now();
        return 3500 - (int)chrono::duration_cast<chrono::milliseconds>(now - start_time).count();
    };

    vector<int> best_path;
    vector<int> path;
    vector<bool> visited(n + 1, false);
    vector<int> u_deg(n + 1, 0);
    path.reserve(n);

    auto reset_state = [&]() {
        fill(visited.begin(), visited.end(), false);
        for (int v = 1; v <= n; v++) u_deg[v] = (int)adj[v].size();
    };

    auto try_warnsdorff = [&](int start) {
        reset_state();
        path.clear();
        path.push_back(start);
        visited[start] = true;
        for (int p : radj[start]) u_deg[p]--;
        int cur = start;
        while (true) {
            int best_next = -1;
            int best_score = INT_MAX;
            for (int v : adj[cur]) {
                if (visited[v]) continue;
                int s = u_deg[v];
                if (s < best_score) {
                    best_score = s;
                    best_next = v;
                }
            }
            if (best_next == -1) break;
            path.push_back(best_next);
            visited[best_next] = true;
            for (int p : radj[best_next]) u_deg[p]--;
            cur = best_next;
        }
        if (path.size() > best_path.size()) best_path = path;
    };

    vector<int> starts;
    starts.reserve(n);
    for (int i = 1; i <= n; i++) starts.push_back(i);
    sort(starts.begin(), starts.end(), [&](int x, int y) {
        return in_deg[x] < in_deg[y];
    });

    for (int s : starts) {
        if (time_left_ms() <= 0) break;
        try_warnsdorff(s);
        if ((int)best_path.size() == n) break;
    }

    cout << best_path.size() << '\n';
    for (size_t i = 0; i < best_path.size(); i++) {
        cout << best_path[i];
        if (i + 1 < best_path.size()) cout << ' ';
    }
    cout << '\n';
    return 0;
}
