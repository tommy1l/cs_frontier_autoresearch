/**
 * Problem 3: Ring lamp arrangement. Interactive.
 * Naive O(n^2) all-pairs test in a single batch query.
 * Targets subtask 1 (n=1000). Subtask 2 (n=10^5) falls back to identity guess.
 */
#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int subtask, n;
    cin >> subtask >> n;

    if (n == 1) {
        cout << -1 << ' ' << 1 << '\n';
        cout.flush();
        return 0;
    }
    if (n == 2) {
        cout << -1 << ' ' << 1 << ' ' << 2 << '\n';
        cout.flush();
        return 0;
    }

    if (n > 1500) {
        // Out-of-budget for naive algorithm; fall back.
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    // Build a single big query that tests every pair (v, k) with v < k.
    // Pattern per v: light v; for each k>v, light k then light k (test then off);
    // then light v (extinguish v). After all v's, S returns to empty.
    vector<int> ops;
    ops.reserve((size_t)n * (size_t)n + (size_t)2 * n + 5);
    for (int v = 1; v <= n; v++) {
        ops.push_back(v);
        for (int k = v + 1; k <= n; k++) {
            ops.push_back(k);
            ops.push_back(k);
        }
        ops.push_back(v);
    }

    cout << (int)ops.size();
    for (int x : ops) cout << ' ' << x;
    cout << '\n';
    cout.flush();

    vector<int> resp(ops.size());
    for (size_t i = 0; i < ops.size(); i++) cin >> resp[i];

    vector<vector<bool>> adj(n + 1, vector<bool>(n + 1, false));
    size_t idx = 0;
    for (int v = 1; v <= n; v++) {
        idx++; // light v response
        for (int k = v + 1; k <= n; k++) {
            int r = resp[idx++]; // S = {v, k}
            if (r) { adj[v][k] = true; adj[k][v] = true; }
            idx++; // extinguish k
        }
        idx++; // extinguish v
    }

    // Trace the ring starting from lamp 1.
    vector<int> perm;
    perm.reserve(n);
    perm.push_back(1);
    vector<bool> used(n + 1, false);
    used[1] = true;
    while ((int)perm.size() < n) {
        int curr = perm.back();
        bool found = false;
        for (int k = 1; k <= n; k++) {
            if (!used[k] && adj[curr][k]) {
                perm.push_back(k);
                used[k] = true;
                found = true;
                break;
            }
        }
        if (!found) {
            // Should not happen if adjacency was correctly decoded.
            for (int k = 1; k <= n; k++) if (!used[k]) {
                perm.push_back(k);
                used[k] = true;
                break;
            }
        }
    }

    cout << -1;
    for (int x : perm) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    return 0;
}
