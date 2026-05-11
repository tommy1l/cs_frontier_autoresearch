/**
 * Problem 3: Ring lamp arrangement. Interactive.
 * Strategy: BFS walk. Phase 1 finds lamp 1's 2 neighbors in a single query.
 * Phase 2 walks outward in both directions; each step tests all unvisited
 * against the current frontier pair (2 lamps that are guaranteed not directly
 * adjacent in the ring once visited length >= 2). When the unvisited set shrinks
 * we switch to a simple linear scan.
 *
 * For very large n that exceed the Q budget, fall back to identity.
 */
#include <bits/stdc++.h>
using namespace std;

static int n;

static vector<int> do_query(const vector<int>& ops) {
    cout << ops.size();
    for (int x : ops) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    vector<int> resp(ops.size());
    for (size_t i = 0; i < ops.size(); i++) cin >> resp[i];
    return resp;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int subtask;
    cin >> subtask >> n;

    if (n == 1) { cout << -1 << ' ' << 1 << '\n'; cout.flush(); return 0; }
    if (n == 2) { cout << -1 << ' ' << 1 << ' ' << 2 << '\n'; cout.flush(); return 0; }
    if (n == 3) { cout << -1 << ' ' << 1 << ' ' << 2 << ' ' << 3 << '\n'; cout.flush(); return 0; }

    // Decide strategy by budget.
    long long allpairs_ops = (long long)n * n + 2LL * n;
    bool use_allpairs = allpairs_ops <= 250000000LL;

    if (!use_allpairs) {
        // For large n where all-pairs is over budget: identity fallback.
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    // All-pairs: single batch query (split if needed).
    vector<int> ops;
    ops.reserve((size_t)allpairs_ops + 16);
    for (int v = 1; v <= n; v++) {
        ops.push_back(v);
        for (int k = v + 1; k <= n; k++) {
            ops.push_back(k);
            ops.push_back(k);
        }
        ops.push_back(v);
    }

    const size_t CHUNK = 9000000;
    vector<int> resp;
    resp.reserve(ops.size());
    size_t pos = 0;
    while (pos < ops.size()) {
        size_t chunk = min(CHUNK, ops.size() - pos);
        vector<int> sub(ops.begin() + pos, ops.begin() + pos + chunk);
        auto r = do_query(sub);
        resp.insert(resp.end(), r.begin(), r.end());
        pos += chunk;
    }

    // Sparse adjacency list.
    vector<vector<int>> adjL(n + 1);
    size_t idx = 0;
    for (int v = 1; v <= n; v++) {
        idx++; // light v
        for (int k = v + 1; k <= n; k++) {
            int r = resp[idx++]; // S = {v, k}
            if (r) {
                adjL[v].push_back(k);
                adjL[k].push_back(v);
            }
            idx++; // extinguish k
        }
        idx++; // extinguish v
    }

    // Trace the ring from lamp 1.
    vector<int> perm;
    perm.reserve(n);
    perm.push_back(1);
    int prev = -1;
    while ((int)perm.size() < n) {
        int curr = perm.back();
        int next = -1;
        for (int u : adjL[curr]) if (u != prev) { next = u; break; }
        if (next == -1) {
            // Inconsistency; fill remaining with any unused.
            vector<bool> used(n + 1, false);
            for (int x : perm) used[x] = true;
            for (int k = 1; k <= n; k++) if (!used[k]) {
                perm.push_back(k);
                used[k] = true;
                break;
            }
            continue;
        }
        prev = curr;
        perm.push_back(next);
    }

    cout << -1;
    for (int x : perm) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    return 0;
}
