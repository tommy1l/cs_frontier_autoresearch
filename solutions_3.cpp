/**
 * Problem 3: Ring lamp arrangement.
 * Hybrid strategy: all-pairs single big query for small n, sequential BFS
 * (per-step queries) for medium n that exceeds single-query budget, identity
 * fallback for very large n.
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

static vector<int> find_nbrs(int v, const vector<int>& cands) {
    vector<int> ops;
    ops.reserve(2 + 2 * cands.size());
    ops.push_back(v);
    for (int k : cands) { ops.push_back(k); ops.push_back(k); }
    ops.push_back(v);
    auto resp = do_query(ops);
    vector<int> nbrs;
    int idx = 1;
    for (size_t i = 0; i < cands.size(); i++) {
        if (resp[idx] == 1) nbrs.push_back(cands[i]);
        idx += 2;
    }
    return nbrs;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int subtask;
    cin >> subtask >> n;

    if (n <= 3) {
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    long long allpairs_ops = (long long)n * (n - 1) + 2LL * n;
    bool use_allpairs = allpairs_ops <= 295000000LL;

    vector<vector<int>> adjL(n + 1);
    auto add_edge = [&](int u, int v) {
        adjL[u].push_back(v);
        adjL[v].push_back(u);
    };

    if (use_allpairs) {
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
        size_t pos = 0;
        vector<int> resp;
        resp.reserve(ops.size());
        while (pos < ops.size()) {
            size_t chunk = min(CHUNK, ops.size() - pos);
            vector<int> sub(ops.begin() + pos, ops.begin() + pos + chunk);
            auto r = do_query(sub);
            resp.insert(resp.end(), r.begin(), r.end());
            pos += chunk;
        }
        size_t idx = 0;
        for (int v = 1; v <= n; v++) {
            idx++;
            for (int k = v + 1; k <= n; k++) {
                int r = resp[idx++];
                if (r) add_edge(v, k);
                idx++;
            }
            idx++;
        }
    } else {
        // Sequential BFS: find lamp 1's 2 neighbors, walk outward.
        // For n where allpairs is over budget but per-step BFS could fit
        // in total Q budget.
        long long bfs_total = (long long)n * n / 2;
        if (bfs_total > 290000000LL) {
            // Even BFS over budget; fall back.
            cout << -1;
            for (int i = 1; i <= n; i++) cout << ' ' << i;
            cout << '\n';
            cout.flush();
            return 0;
        }

        vector<int> cands;
        for (int k = 2; k <= n; k++) cands.push_back(k);
        auto nbrs1 = find_nbrs(1, cands);
        if (nbrs1.size() != 2) {
            cout << -1;
            for (int i = 1; i <= n; i++) cout << ' ' << i;
            cout << '\n';
            cout.flush();
            return 0;
        }
        add_edge(1, nbrs1[0]);
        add_edge(1, nbrs1[1]);

        // BFS in forward direction only (the ring closes back).
        vector<bool> used(n + 1, false);
        used[1] = used[nbrs1[0]] = used[nbrs1[1]] = true;
        int curr = nbrs1[0];
        int prev = 1;
        while (true) {
            // Find curr's other neighbor.
            vector<int> remain;
            for (int k = 1; k <= n; k++) if (!used[k] && k != curr) remain.push_back(k);
            if (remain.empty()) break;
            auto nbrs = find_nbrs(curr, remain);
            int next_lamp = -1;
            for (int u : nbrs) {
                if (u != prev) { next_lamp = u; break; }
            }
            // If next_lamp would close ring (== nbrs1[1] which is back side)
            // then we're done.
            if (next_lamp == nbrs1[1] || next_lamp == -1) {
                if (next_lamp != -1) add_edge(curr, next_lamp);
                break;
            }
            add_edge(curr, next_lamp);
            used[next_lamp] = true;
            prev = curr;
            curr = next_lamp;
        }
    }

    // Trace ring from lamp 1; pick the smaller-labeled neighbor first to make
    // output deterministic.
    vector<int> perm;
    perm.reserve(n);
    perm.push_back(1);
    int prev = -1;
    vector<bool> used(n + 1, false);
    used[1] = true;
    while ((int)perm.size() < n) {
        int curr = perm.back();
        int next = -1;
        int best_label = INT_MAX;
        for (int u : adjL[curr]) {
            if (u != prev && !used[u] && u < best_label) {
                best_label = u;
                next = u;
            }
        }
        if (next == -1) {
            for (int k = 1; k <= n; k++) if (!used[k]) { next = k; break; }
        }
        if (next == -1) break;
        prev = curr;
        perm.push_back(next);
        used[next] = true;
    }
    while ((int)perm.size() < n) {
        for (int k = 1; k <= n; k++) if (!used[k]) { perm.push_back(k); used[k] = true; break; }
    }

    cout << -1;
    for (int x : perm) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    return 0;
}
