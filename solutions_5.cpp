/**
 * Problem 5: longest simple path in a directed graph.
 *
 * If the graph is a DAG, find the optimum via topological sort + DP. Otherwise
 * fall back to bidirectional greedy + random-walk restarts.
 */
#include <bits/stdc++.h>
using namespace std;

int n, m;
vector<vector<int>> adj, radj;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    vector<int> a(10);
    for (int i = 0; i < 10; i++) cin >> a[i];
    adj.assign(n + 1, {});
    radj.assign(n + 1, {});
    vector<int> indeg(n + 1, 0);
    for (int i = 0; i < m; i++) {
        int u, v; cin >> u >> v;
        adj[u].push_back(v);
        radj[v].push_back(u);
        indeg[v]++;
    }

    auto start_time = chrono::steady_clock::now();
    auto time_left_ms = [&]() {
        auto now = chrono::steady_clock::now();
        return 3500 - (int)chrono::duration_cast<chrono::milliseconds>(now - start_time).count();
    };

    vector<int> best_path;

    // DAG branch: topo sort + longest-path DP.
    {
        vector<int> indeg_copy = indeg;
        vector<int> topo;
        topo.reserve(n);
        queue<int> q;
        for (int i = 1; i <= n; i++) if (indeg_copy[i] == 0) q.push(i);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            topo.push_back(u);
            for (int v : adj[u]) {
                if (--indeg_copy[v] == 0) q.push(v);
            }
        }
        if ((int)topo.size() == n) {
            // Forward DP: longest path starting at u.
            vector<int> fwd(n + 1, 1);
            vector<int> nxt(n + 1, -1);
            for (int i = n - 1; i >= 0; i--) {
                int u = topo[i];
                for (int v : adj[u]) {
                    if (fwd[v] + 1 > fwd[u]) { fwd[u] = fwd[v] + 1; nxt[u] = v; }
                }
            }
            // Backward DP: longest path ending at u.
            vector<int> bwd(n + 1, 1);
            vector<int> prv(n + 1, -1);
            for (int u : topo) {
                for (int v : adj[u]) {
                    if (bwd[u] + 1 > bwd[v]) { bwd[v] = bwd[u] + 1; prv[v] = u; }
                }
            }
            // Longest path through any vertex: bwd[v] + fwd[v] - 1.
            int best_v = 1, best_len = 0;
            for (int v = 1; v <= n; v++) {
                int len = bwd[v] + fwd[v] - 1;
                if (len > best_len) { best_len = len; best_v = v; }
            }
            vector<int> left_part, right_part;
            for (int u = best_v; u != -1; u = prv[u]) left_part.push_back(u);
            reverse(left_part.begin(), left_part.end());
            for (int u = nxt[best_v]; u != -1; u = nxt[u]) right_part.push_back(u);
            vector<int> path = left_part;
            for (int x : right_part) path.push_back(x);
            if (path.size() > best_path.size()) best_path = path;
        }
    }

    if ((int)best_path.size() < n) {
        vector<int> vgen(n + 1, 0);
        int gen = 0;

        auto try_bi = [&](int start, bool randomize, mt19937* rng) {
            gen++;
            deque<int> dq;
            dq.push_back(start);
            vgen[start] = gen;
            int cur = start;
            while (true) {
                int next = -1;
                if (randomize) {
                    int count = 0;
                    for (int v : adj[cur]) {
                        if (vgen[v] == gen) continue;
                        count++;
                        if ((int)((*rng)() % count) == 0) next = v;
                    }
                } else {
                    for (int v : adj[cur]) { if (vgen[v] != gen) { next = v; break; } }
                }
                if (next == -1) break;
                dq.push_back(next);
                vgen[next] = gen;
                cur = next;
            }
            cur = dq.front();
            while (true) {
                int prev = -1;
                if (randomize) {
                    int count = 0;
                    for (int u : radj[cur]) {
                        if (vgen[u] == gen) continue;
                        count++;
                        if ((int)((*rng)() % count) == 0) prev = u;
                    }
                } else {
                    for (int u : radj[cur]) { if (vgen[u] != gen) { prev = u; break; } }
                }
                if (prev == -1) break;
                dq.push_front(prev);
                vgen[prev] = gen;
                cur = prev;
            }
            if ((int)dq.size() > (int)best_path.size()) {
                best_path.assign(dq.begin(), dq.end());
            }
        };

        mt19937 rng(42);
        vector<int> starts;
        starts.reserve(n);
        for (int i = 1; i <= n; i++) starts.push_back(i);
        shuffle(starts.begin(), starts.end(), rng);

        for (int s : starts) {
            if (time_left_ms() <= 0) break;
            try_bi(s, false, nullptr);
            if ((int)best_path.size() == n) goto done;
        }
        while (time_left_ms() > 50) {
            int s = starts[rng() % starts.size()];
            try_bi(s, true, &rng);
            if ((int)best_path.size() == n) goto done;
        }
    }

done:
    cout << best_path.size() << '\n';
    for (size_t i = 0; i < best_path.size(); i++) {
        cout << best_path[i];
        if (i + 1 < best_path.size()) cout << ' ';
    }
    cout << '\n';
    return 0;
}
