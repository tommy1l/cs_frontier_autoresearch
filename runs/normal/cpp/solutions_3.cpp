/**
 * Problem 3: Ring lamp arrangement.
 * All-pairs O(n^2) probe in chunked queries for n where it fits the op budget;
 * identity fallback otherwise. Simplified: removed the never-triggered
 * sequential-BFS middle path (no test n falls in that band).
 */
#include <bits/stdc++.h>
using namespace std;

static int n;

static vector<int> do_query(const vector<int>& ops) {
    string buf;
    buf.reserve(16 * ops.size());
    buf += to_string(ops.size());
    for (int x : ops) { buf += ' '; buf += to_string(x); }
    buf += '\n';
    cout << buf;
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

    if (n <= 3) {
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    long long allpairs_ops = (long long)n * (n - 1) + 2LL * n;
    bool use_allpairs = allpairs_ops <= 270000000LL;

    if (!use_allpairs) {
        // No feasible exact algorithm within budget for very large n.
        cout << -1;
        for (int i = 1; i <= n; i++) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    vector<vector<int>> adjL(n + 1);
    auto add_edge = [&](int u, int v) {
        adjL[u].push_back(v);
        adjL[v].push_back(u);
    };

    // Light v, probe every k>v with a toggle-pair, extinguish v. One query
    // per chunk; the lamp under test is the only lit lamp during each probe.
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
    const size_t CHUNK = 8500000;
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

    // Trace the ring starting from lamp 1.
    vector<int> perm;
    perm.reserve(n);
    perm.push_back(1);
    int prev = -1;
    vector<bool> used(n + 1, false);
    used[1] = true;
    while ((int)perm.size() < n) {
        int curr = perm.back();
        int next = -1;
        for (int u : adjL[curr]) {
            if (u != prev && !used[u]) { next = u; break; }
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
