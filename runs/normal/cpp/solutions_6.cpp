/**
 * Problem 6: worldmap (IOI 2024).
 *
 * Variable-width diagonal stripes.
 *   - Find a Hamiltonian path of G (heuristic backtracking) or fall back to a
 *     DFS-tree walk. Call this `walk` (length L).
 *   - The non-walk subgraph G' contains every G-edge that isn't witnessed by
 *     consecutive walk positions. A vertex needs a *middle* (intrusion) diagonal
 *     only if some non-walk edge has to be placed at it. Pick an independent set
 *     I of G' (no two share a non-walk edge); vertices in I get width 1, others
 *     get width 3. We use a min-degree-first greedy for I.
 *   - Total diagonals D = 3 * |cover| + 1 * |I| = 3N - 2|I|; the grid is K x K
 *     with K = ceil((D+1)/2).
 *   - Cell (r, c) takes color walk[diag_to_walk_pos[r+c]]. Adjacent cells differ
 *     in d by 1, so colour changes only at width-boundaries between consecutive
 *     walk positions — those are walk edges and therefore G-edges.
 *   - A width-3 position p has a middle diagonal at cum_w[p]+1; both neighbour
 *     diagonals carry walk[p], so single-cell intrusions there only witness
 *     (walk[p], intruder), no spurious adjacency.
 *   - For each non-walk edge (u, v), assign to either u's or v's middle (the
 *     endpoint that's in the cover). For each `forced` edge (only one endpoint
 *     in the cover) we place it first; for `free` edges we greedily place at
 *     whichever endpoint has more remaining mid-diagonal capacity.
 */
#include <bits/stdc++.h>
using namespace std;

static chrono::steady_clock::time_point t_start;

static bool time_left(int ms) {
    auto now = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - t_start).count() < ms;
}

static bool ham_dfs(int u, int N, const vector<vector<int>>& nbr,
                    vector<char>& vis, vector<int>& path) {
    path.push_back(u);
    vis[u] = 1;
    if ((int)path.size() == N) return true;
    if (!time_left(2500)) {
        path.pop_back();
        vis[u] = 0;
        return false;
    }
    for (int v : nbr[u]) {
        if (!vis[v]) {
            if (ham_dfs(v, N, nbr, vis, path)) return true;
        }
    }
    path.pop_back();
    vis[u] = 0;
    return false;
}

static vector<int> find_ham(int N, const vector<vector<int>>& nbr) {
    vector<int> order(N);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return nbr[a].size() < nbr[b].size();
    });
    for (int start : order) {
        vector<int> path;
        vector<char> vis(N + 1, 0);
        if (ham_dfs(start, N, nbr, vis, path)) return path;
        if (!time_left(2500)) break;
    }
    return {};
}

static vector<int> dfs_walk(int N, const vector<vector<int>>& nbr, int root) {
    vector<int> walk;
    vector<char> vis(N + 1, 0);
    function<void(int)> dfs = [&](int u) {
        walk.push_back(u);
        vis[u] = 1;
        for (int v : nbr[u]) {
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
    vector<vector<int>> nbr(N + 1);
    for (int i = 0; i < M; i++) {
        int x = A[i], y = B[i];
        if (!adj[x][y]) {
            adj[x][y] = adj[y][x] = 1;
            nbr[x].push_back(y);
            nbr[y].push_back(x);
        }
    }
    for (int v = 1; v <= N; v++) sort(nbr[v].begin(), nbr[v].end());

    t_start = chrono::steady_clock::now();
    vector<int> walk = find_ham(N, nbr);
    if ((int)walk.size() != N) {
        int root = 1;
        for (int v = 1; v <= N; v++) if (!nbr[v].empty()) { root = v; break; }
        walk = dfs_walk(N, nbr, root);
    }
    int L = (int)walk.size();

    vector<vector<char>> covered(N + 1, vector<char>(N + 1, 0));
    for (int i = 0; i + 1 < L; i++) {
        covered[walk[i]][walk[i + 1]] = 1;
        covered[walk[i + 1]][walk[i]] = 1;
    }

    // Non-walk-edge subgraph
    vector<vector<int>> nw_nbr(N + 1);
    for (int u = 1; u <= N; u++)
        for (int v = u + 1; v <= N; v++)
            if (adj[u][v] && !covered[u][v]) {
                nw_nbr[u].push_back(v);
                nw_nbr[v].push_back(u);
            }

    // Greedy max IS in non-walk subgraph (min-degree-first).
    vector<char> in_is(N + 1, 0);
    vector<char> avail(N + 1, 1);
    while (true) {
        int best = -1, best_d = INT_MAX;
        for (int v = 1; v <= N; v++) {
            if (!avail[v]) continue;
            int d = 0;
            for (int u : nw_nbr[v]) if (avail[u]) d++;
            if (d < best_d) { best_d = d; best = v; }
        }
        if (best == -1) break;
        in_is[best] = 1;
        avail[best] = 0;
        for (int u : nw_nbr[best]) avail[u] = 0;
    }

    vector<int> width(L);
    for (int p = 0; p < L; p++) width[p] = in_is[walk[p]] ? 1 : 3;

    vector<int> cum_w(L + 1, 0);
    for (int p = 0; p < L; p++) cum_w[p + 1] = cum_w[p] + width[p];
    int D = cum_w[L];
    int K = (D + 2) / 2;
    if (K > 240) K = 240;
    if (K < 1) K = 1;

    vector<int> diag_pos(D);
    {
        int idx = 0;
        for (int p = 0; p < L; p++)
            for (int j = 0; j < width[p]; j++) diag_pos[idx++] = p;
    }

    vector<vector<int>> grid(K, vector<int>(K));
    for (int r = 0; r < K; r++) {
        for (int c = 0; c < K; c++) {
            int d = r + c;
            if (d >= D) d = D - 1;
            grid[r][c] = walk[diag_pos[d]];
        }
    }

    vector<vector<pair<int, int>>> mid_cells(L);
    for (int p = 0; p < L; p++) {
        if (width[p] != 3) continue;
        int d = cum_w[p] + 1;
        for (int r = 0; r < K; r++) {
            int c = d - r;
            if (c >= 0 && c < K) mid_cells[p].push_back({r, c});
        }
    }
    vector<int> used(L, 0);

    auto place = [&](int host, int color) -> bool {
        int best_p = -1, best_rem = -1;
        for (int p = 0; p < L; p++) {
            if (walk[p] != host || width[p] != 3) continue;
            int rem = (int)mid_cells[p].size() - used[p];
            if (rem > best_rem) { best_rem = rem; best_p = p; }
        }
        if (best_p == -1 || best_rem <= 0) return false;
        auto [r, c] = mid_cells[best_p][used[best_p]];
        grid[r][c] = color;
        used[best_p]++;
        return true;
    };

    // Forced edges first (exactly one endpoint in cover).
    for (int u = 1; u <= N; u++) {
        for (int v = u + 1; v <= N; v++) {
            if (!adj[u][v] || covered[u][v]) continue;
            bool u_cov = !in_is[u], v_cov = !in_is[v];
            if (u_cov == v_cov) continue;
            int host = u_cov ? u : v;
            int color = u_cov ? v : u;
            if (place(host, color)) covered[u][v] = covered[v][u] = 1;
        }
    }

    // Free edges (both endpoints in cover): place at endpoint with more capacity.
    for (int u = 1; u <= N; u++) {
        for (int v = u + 1; v <= N; v++) {
            if (!adj[u][v] || covered[u][v]) continue;
            int best_p = -1, best_rem = -1, color = -1;
            for (int p = 0; p < L; p++) {
                if (walk[p] != u && walk[p] != v) continue;
                if (width[p] != 3) continue;
                int rem = (int)mid_cells[p].size() - used[p];
                if (rem > best_rem) {
                    best_rem = rem;
                    best_p = p;
                    color = (walk[p] == u) ? v : u;
                }
            }
            if (best_p == -1 || best_rem <= 0) continue;
            auto [r, c] = mid_cells[best_p][used[best_p]];
            grid[r][c] = color;
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
