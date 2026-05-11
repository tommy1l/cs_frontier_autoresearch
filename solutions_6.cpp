/**
 * Problem 6: worldmap (IOI 2024).
 *
 * Diagonal construction with 3 anti-diagonals per walk position.
 *   - Try to find a Hamiltonian path of G (heuristic backtracking with a time budget).
 *     If found, walk length L = N and K = ceil((3N+1)/2) ~ 1.5 N.
 *   - Otherwise fall back to a DFS-tree walk of length 2N-1.
 *   - Cell (r,c) on anti-diagonal d=r+c gets color walk[d/3]. Two neighbouring cells
 *     differ in d by 1, so a colour change only happens at d = 3p+2 / 3p+3 boundary,
 *     i.e. between walk[p] and walk[p+1] — those are consecutive in the walk so the
 *     adjacency is a legal G-edge.
 *   - For each non-walk edge (a,b), drop a single-cell intrusion on the middle
 *     diagonal d = 3p+1 of some walk-position p with walk[p] in {a,b}. Both its
 *     diagonal neighbours (d-1, d+1) carry colour walk[p], so the intrusion only
 *     witnesses (walk[p], other-endpoint).
 *   - Multiple intrusions can sit on the same middle diagonal: cells (r,c) and
 *     (r-1,c+1) share no grid side, so they don't create spurious cross-colour
 *     adjacencies even if both are intrusions of different colours.
 *
 * Capacity per middle diagonal is up to K cells, well above the per-vertex degree
 * cap of N-1 = 39 for N <= 40. We greedily place each non-walk edge on whichever
 * endpoint's middle diagonal currently has more remaining capacity.
 */
#include <bits/stdc++.h>
using namespace std;

static chrono::steady_clock::time_point t_start;

static bool time_left(int ms) {
    auto now = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - t_start).count() < ms;
}

static bool ham_dfs(int u, int N, const vector<vector<int>>& adj_list,
                    vector<char>& vis, vector<int>& path) {
    path.push_back(u);
    vis[u] = 1;
    if ((int)path.size() == N) return true;
    if (!time_left(2500)) {
        path.pop_back();
        vis[u] = 0;
        return false;
    }
    for (int v : adj_list[u]) {
        if (!vis[v]) {
            if (ham_dfs(v, N, adj_list, vis, path)) return true;
        }
    }
    path.pop_back();
    vis[u] = 0;
    return false;
}

static vector<int> find_ham(int N, const vector<vector<int>>& adj_list) {
    // Try starts in increasing degree order — low-degree vertices are more constrained.
    vector<int> order(N);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return adj_list[a].size() < adj_list[b].size();
    });
    for (int start : order) {
        vector<int> path;
        vector<char> vis(N + 1, 0);
        if (ham_dfs(start, N, adj_list, vis, path)) return path;
        if (!time_left(2500)) break;
    }
    return {};
}

static vector<int> dfs_walk(int N, const vector<vector<int>>& adj_list, int root = 1) {
    vector<int> walk;
    vector<char> vis(N + 1, 0);
    function<void(int)> dfs = [&](int u) {
        walk.push_back(u);
        vis[u] = 1;
        for (int v : adj_list[u]) {
            if (!vis[v]) {
                dfs(v);
                walk.push_back(u);
            }
        }
    };
    dfs(root);
    return walk;
}

vector<vector<int>> create_map(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};

    vector<vector<char>> adj(N + 1, vector<char>(N + 1, 0));
    vector<vector<int>> adj_list(N + 1);
    for (int i = 0; i < M; i++) {
        int x = A[i], y = B[i];
        if (!adj[x][y]) {
            adj[x][y] = adj[y][x] = 1;
            adj_list[x].push_back(y);
            adj_list[y].push_back(x);
        }
    }
    for (int v = 1; v <= N; v++) sort(adj_list[v].begin(), adj_list[v].end());

    t_start = chrono::steady_clock::now();
    vector<int> walk = find_ham(N, adj_list);
    if ((int)walk.size() != N) {
        // Pick a root from the largest component (just use vertex 1 if it has edges).
        int root = 1;
        for (int v = 1; v <= N; v++) if (!adj_list[v].empty()) { root = v; break; }
        walk = dfs_walk(N, adj_list, root);
    }

    int L = (int)walk.size();
    int K = (3 * L + 2) / 2;
    if (K < 1) K = 1;
    if (K > 240) K = 240;

    vector<vector<int>> grid(K, vector<int>(K, walk[0]));
    for (int r = 0; r < K; r++) {
        for (int c = 0; c < K; c++) {
            int d = r + c;
            int idx = d / 3;
            if (idx >= L) idx = L - 1;
            grid[r][c] = walk[idx];
        }
    }

    vector<vector<char>> covered(N + 1, vector<char>(N + 1, 0));
    for (int i = 0; i + 1 < L; i++) {
        covered[walk[i]][walk[i + 1]] = 1;
        covered[walk[i + 1]][walk[i]] = 1;
    }

    vector<vector<pair<int, int>>> mid_cells(L);
    for (int p = 0; p < L; p++) {
        int d = 3 * p + 1;
        for (int r = 0; r < K; r++) {
            int c = d - r;
            if (c >= 0 && c < K) mid_cells[p].push_back({r, c});
        }
    }
    vector<int> used(L, 0);

    for (int u = 1; u <= N; u++) {
        for (int v = u + 1; v <= N; v++) {
            if (!adj[u][v] || covered[u][v]) continue;
            int best_p = -1, intrude_color = -1, best_rem = -1;
            for (int p = 0; p < L; p++) {
                if (walk[p] != u && walk[p] != v) continue;
                int rem = (int)mid_cells[p].size() - used[p];
                if (rem > best_rem) {
                    best_rem = rem;
                    best_p = p;
                    intrude_color = (walk[p] == u) ? v : u;
                }
            }
            if (best_p == -1 || best_rem <= 0) continue;
            auto [r, c] = mid_cells[best_p][used[best_p]];
            grid[r][c] = intrude_color;
            used[best_p]++;
            covered[u][v] = covered[v][u] = 1;
        }
    }

    return grid;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int N, M;
        cin >> N >> M;
        vector<int> A(M), B(M);
        for (int i = 0; i < M; i++) cin >> A[i] >> B[i];
        auto C = create_map(N, M, A, B);
        int P = (int)C.size();
        cout << P << "\n";
        for (int i = 0; i < P; i++) {
            cout << C[i].size();
            cout << (i + 1 < P ? ' ' : '\n');
        }
        cout << "\n";
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < (int)C[i].size(); j++) {
                cout << C[i][j];
                cout << (j + 1 < (int)C[i].size() ? ' ' : '\n');
            }
        }
    }
    return 0;
}
