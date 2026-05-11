/**
 * Problem 5: longest simple path in a directed graph (a Hamiltonian path is
 * guaranteed to exist; we just need a path as long as possible).
 *
 * Baseline: greedy walk from many starting vertices; pick the longest path
 * found. Try the original adjacency order, then with shuffled adjacency.
 */
#include <bits/stdc++.h>
using namespace std;

int n, m;
vector<vector<int>> adj;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    vector<int> a(10);
    for (int i = 0; i < 10; i++) cin >> a[i];
    adj.assign(n + 1, {});
    for (int i = 0; i < m; i++) {
        int u, v; cin >> u >> v;
        adj[u].push_back(v);
    }

    auto start_time = chrono::steady_clock::now();
    auto time_left_ms = [&]() {
        auto now = chrono::steady_clock::now();
        return 3500 - (int)chrono::duration_cast<chrono::milliseconds>(now - start_time).count();
    };

    vector<int> best_path;
    vector<int> path;
    vector<bool> visited(n + 1, false);
    path.reserve(n);

    auto try_greedy = [&](int start) {
        fill(visited.begin(), visited.end(), false);
        path.clear();
        path.push_back(start);
        visited[start] = true;
        int cur = start;
        while (true) {
            int next = -1;
            for (int v : adj[cur]) {
                if (!visited[v]) { next = v; break; }
            }
            if (next == -1) break;
            path.push_back(next);
            visited[next] = true;
            cur = next;
        }
        if (path.size() > best_path.size()) best_path = path;
    };

    mt19937 rng(42);
    vector<int> starts;
    starts.reserve(n);
    for (int i = 1; i <= n; i++) starts.push_back(i);
    shuffle(starts.begin(), starts.end(), rng);

    for (int s : starts) {
        if (time_left_ms() <= 0) break;
        try_greedy(s);
        if ((int)best_path.size() == n) break;
    }

    if ((int)best_path.size() < n) {
        for (int i = 1; i <= n; i++) shuffle(adj[i].begin(), adj[i].end(), rng);
        for (int s : starts) {
            if (time_left_ms() <= 0) break;
            try_greedy(s);
            if ((int)best_path.size() == n) break;
        }
    }

    cout << best_path.size() << '\n';
    for (size_t i = 0; i < best_path.size(); i++) {
        cout << best_path[i];
        if (i + 1 < best_path.size()) cout << ' ';
    }
    cout << '\n';
    return 0;
}
