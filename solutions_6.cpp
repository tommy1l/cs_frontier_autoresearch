/**
 * Problem 6: worldmap (IOI 2024).
 *
 * Construct a K x K grid coloring of N countries given the adjacency graph G:
 *   - every color 1..N appears at least once,
 *   - every G-edge (a,b) appears as an adjacent cell pair,
 *   - every adjacent-cell pair with different colors is a G-edge.
 *
 * Strategy (baseline):
 *   - Do a DFS walk of a spanning tree of G from vertex 1. Walk length L <= 2N-1.
 *   - Make K = 3 * L. Use 3-row horizontal bands; band b is solid color walk[b].
 *     Adjacent bands are tree-edge endpoints, so vertical band boundaries are valid.
 *   - For each non-tree edge (a,b), drop a single-cell "intrusion" of color b
 *     into the middle row of some band whose color is a (or symmetric). The
 *     intrusion is surrounded on all 4 sides by color a, creating exactly the
 *     witness adjacency (a,b) and nothing else.
 *
 * Intrusions are placed at odd columns to guarantee single-cell separation.
 * Works for connected G; isolated/disconnected pieces are not handled here.
 */
#include <bits/stdc++.h>
using namespace std;

vector<vector<int>> create_map(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};

    vector<vector<bool>> adj(N + 1, vector<bool>(N + 1, false));
    for (int i = 0; i < M; i++) {
        adj[A[i]][B[i]] = true;
        adj[B[i]][A[i]] = true;
    }

    // DFS walk of a spanning tree, rooted at the smallest-index vertex with any edge.
    int root = 1;
    for (int v = 1; v <= N; v++) {
        bool has_edge = false;
        for (int u = 1; u <= N; u++) if (adj[v][u]) { has_edge = true; break; }
        if (has_edge) { root = v; break; }
    }

    vector<int> walk;
    vector<bool> vis(N + 1, false);
    function<void(int)> dfs = [&](int u) {
        walk.push_back(u);
        vis[u] = true;
        for (int v = 1; v <= N; v++) {
            if (adj[u][v] && !vis[v]) {
                dfs(v);
                walk.push_back(u);
            }
        }
    };
    dfs(root);

    int L = (int)walk.size();
    int K = 3 * L;
    if (K < 1) K = 1;
    if (K > 240) K = 240;

    // Solid 3-row bands.
    vector<vector<int>> grid(K, vector<int>(K));
    for (int r = 0; r < K; r++) {
        int band = min(r / 3, L - 1);
        int color = walk[band];
        for (int c = 0; c < K; c++) grid[r][c] = color;
    }

    // Mark edges covered by adjacent bands.
    vector<vector<bool>> covered(N + 1, vector<bool>(N + 1, false));
    for (int i = 0; i + 1 < L; i++) {
        covered[walk[i]][walk[i + 1]] = true;
        covered[walk[i + 1]][walk[i]] = true;
    }

    // Place intrusions for the remaining edges.
    vector<int> next_col(L, 1);
    for (int a = 1; a <= N; a++) {
        for (int b = a + 1; b <= N; b++) {
            if (!adj[a][b] || covered[a][b]) continue;
            int chosen = -1, intrude = -1;
            for (int band = 0; band < L; band++) {
                if (next_col[band] + 1 >= K) continue;
                if (walk[band] == a) { chosen = band; intrude = b; break; }
                if (walk[band] == b) { chosen = band; intrude = a; break; }
            }
            if (chosen == -1) continue;
            int c = next_col[chosen];
            grid[3 * chosen + 1][c] = intrude;
            next_col[chosen] = c + 2;
            covered[a][b] = covered[b][a] = true;
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
