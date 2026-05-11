#include <bits/stdc++.h>
using namespace std;

// Iter 13: Multi-length Rep state with end-accept via direct edges + post-Hopcroft.
// State = sorted list of (B, Lo, Hi), B >= 1. Two states equivalent iff same set.

struct Rep {
    vector<tuple<int,int,int>> v;
    bool operator<(const Rep& o) const { return v < o.v; }
};

map<Rep, int> mem;
vector<vector<pair<int,int>>> edges;
int endNode;

pair<Rep,bool> trans(const Rep& r, int b) {
    Rep next;
    bool end_accept = false;
    for (auto& [B, Lo, Hi] : r.v) {
        int half = 1 << (B - 1);
        int loB, hiB;
        if (b == 0) {
            loB = Lo;
            hiB = min(Hi, half - 1);
        } else {
            loB = max(Lo, half) - half;
            hiB = Hi - half;
        }
        if (loB > hiB) continue;
        if (B == 1) end_accept = true;
        else next.v.push_back({B - 1, loB, hiB});
    }
    return {next, end_accept};
}

int build(const Rep& r) {
    auto it = mem.find(r);
    if (it != mem.end()) return it->second;
    int sid = (int)edges.size();
    mem[r] = sid;
    edges.push_back({});
    for (int b = 0; b < 2; b++) {
        auto [nr, ea] = trans(r, b);
        if (ea) edges[sid].push_back({endNode, b});
        if (!nr.v.empty()) {
            int child = build(nr);
            edges[sid].push_back({child, b});
        }
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

    endNode = 0;
    edges.push_back({});

    Rep init;
    bool init_end = false;
    for (int B = bL; B <= bR; B++) {
        int half = 1 << (B - 1);
        int lo = max(L, half) - half;
        int hi = min(R, (1 << B) - 1) - half;
        if (lo > hi) continue;
        if (B == 1) init_end = true;
        else init.v.push_back({B - 1, lo, hi});
    }

    int start = (int)edges.size();
    edges.push_back({});
    if (init_end) edges[start].push_back({endNode, 1});
    if (!init.v.empty()) {
        int child = build(init);
        edges[start].push_back({child, 1});
    }

    int n = (int)edges.size();

    // Post-Hopcroft: topological sort, then bottom-up signature merge.
    vector<int> indeg(n, 0);
    for (int i = 0; i < n; i++)
        for (auto& e : edges[i]) indeg[e.first]++;
    queue<int> q;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) q.push(i);
    vector<int> topo;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        topo.push_back(u);
        for (auto& e : edges[u]) if (--indeg[e.first] == 0) q.push(e.first);
    }

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
        for (auto& e : edges[u]) cedges[cid].push_back({canon[e.first], e.second});
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
    for (int i = 0; i < nc; i++)
        for (auto& e : cedges[i])
            out[perm[i]].push_back({perm[e.first], e.second});

    cout << nc << "\n";
    for (int i = 0; i < nc; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) cout << " " << (e.first + 1) << " " << e.second;
        cout << "\n";
    }
    return 0;
}
