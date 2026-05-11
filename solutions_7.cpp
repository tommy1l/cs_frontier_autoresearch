#include <bits/stdc++.h>
using namespace std;

// Iter 12: Build (B,Lo,Hi) DAFSA, then perform DAG minimization by node-signature.

map<tuple<int,int,int>, int> mem;
vector<vector<pair<int,int>>> edges;

int build(int B, int Lo, int Hi) {
    auto key = make_tuple(B, Lo, Hi);
    auto it = mem.find(key);
    if (it != mem.end()) return it->second;
    int sid = (int)edges.size();
    mem[key] = sid;
    edges.push_back({});
    if (B == 0) return sid;
    int mid = 1 << (B - 1);
    int Lo0 = Lo, Hi0 = min(Hi, mid - 1);
    if (Lo0 <= Hi0) {
        int child = build(B - 1, Lo0, Hi0);
        edges[sid].push_back({child, 0});
    }
    if (Hi >= mid) {
        int Lo1 = max(Lo, mid) - mid;
        int Hi1 = Hi - mid;
        int child = build(B - 1, Lo1, Hi1);
        edges[sid].push_back({child, 1});
    }
    return sid;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int L, R;
    cin >> L >> R;

    int bL = 0, bR = 0;
    for (int x = L; x; x >>= 1) bL++;
    for (int x = R; x; x >>= 1) bR++;

    int start = (int)edges.size();
    edges.push_back({});

    if (bL == bR) {
        int B = bL;
        int mid = 1 << (B - 1);
        int child = build(B - 1, L - mid, R - mid);
        edges[start].push_back({child, 1});
    } else {
        int midL = 1 << (bL - 1);
        int childL = build(bL - 1, L - midL, midL - 1);
        edges[start].push_back({childL, 1});
        for (int B = bL + 1; B < bR; B++) {
            int childM = build(B - 1, 0, (1 << (B-1)) - 1);
            edges[start].push_back({childM, 1});
        }
        int midR = 1 << (bR - 1);
        int childR = build(bR - 1, 0, R - midR);
        edges[start].push_back({childR, 1});
    }

    int n = (int)edges.size();

    // Kahn topological sort.
    vector<int> indeg(n, 0);
    for (int i = 0; i < n; i++) {
        for (auto& e : edges[i]) indeg[e.first]++;
    }
    queue<int> q;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) q.push(i);
    vector<int> topo;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        topo.push_back(u);
        for (auto& e : edges[u]) {
            if (--indeg[e.first] == 0) q.push(e.first);
        }
    }

    // Process in reverse topological order (children-first) to compute signatures.
    vector<int> canon(n, -1);
    map<vector<pair<int,int>>, int> sigToId;
    int nextCanon = 0;
    for (int i = (int)topo.size() - 1; i >= 0; i--) {
        int u = topo[i];
        vector<pair<int,int>> sig;
        for (auto& e : edges[u]) sig.push_back({e.second, canon[e.first]});
        sort(sig.begin(), sig.end());
        auto it = sigToId.find(sig);
        if (it == sigToId.end()) {
            canon[u] = nextCanon++;
            sigToId[sig] = canon[u];
        } else {
            canon[u] = it->second;
        }
    }

    int nc = nextCanon;
    vector<vector<pair<int,int>>> cedges(nc);
    vector<bool> seen(nc, false);
    for (int u = 0; u < n; u++) {
        int cid = canon[u];
        if (seen[cid]) continue;
        seen[cid] = true;
        for (auto& e : edges[u]) {
            cedges[cid].push_back({canon[e.first], e.second});
        }
    }

    int cstart = canon[start];
    vector<int> perm(nc);
    perm[cstart] = 0;
    int idx = 1;
    for (int i = 0; i < nc; i++) {
        if (i == cstart) continue;
        perm[i] = idx++;
    }
    vector<vector<pair<int,int>>> out(nc);
    for (int i = 0; i < nc; i++) {
        for (auto& e : cedges[i]) {
            out[perm[i]].push_back({perm[e.first], e.second});
        }
    }
    cout << nc << "\n";
    for (int i = 0; i < nc; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
