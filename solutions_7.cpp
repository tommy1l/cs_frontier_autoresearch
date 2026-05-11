#include <bits/stdc++.h>
using namespace std;

// Min-DFA with multi-length states; end-accept handled via direct edges to end.
// State Rep = sorted list of (B, Lo, Hi) with B >= 1, at most one per B.
// Future language of state = union over (B, Lo, Hi) of length-B strings in [Lo, Hi].

struct Rep {
    // sorted by B
    vector<tuple<int,int,int>> v;
    bool operator<(const Rep& o) const { return v < o.v; }
};

map<Rep, int> mem;
vector<vector<pair<int,int>>> edges;
int endNode;

// transition: given rep and bit b, return next_rep and whether end-accept fires.
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
        if (B == 1) {
            end_accept = true;
        } else {
            next.v.push_back({B - 1, loB, hiB});
        }
    }
    // already sorted by B since input was sorted
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
        if (ea) {
            edges[sid].push_back({endNode, b});
        }
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

    // Pre-allocate end node.
    endNode = 0;
    edges.push_back({});

    // Initial rep after leading 1.
    Rep init;
    bool init_end = false;
    for (int B = bL; B <= bR; B++) {
        int half = 1 << (B - 1);
        int lo = max(L, half) - half;
        int hi = min(R, (1 << B) - 1) - half;
        if (lo > hi) continue;
        if (B == 1) {
            init_end = true;
        } else {
            init.v.push_back({B - 1, lo, hi});
        }
    }

    int start = (int)edges.size();
    edges.push_back({});
    if (init_end) {
        edges[start].push_back({endNode, 1});
    }
    if (!init.v.empty()) {
        int child = build(init);
        edges[start].push_back({child, 1});
    }

    // Reorder so start = 0, end = last (problem doesn't require, but cleaner).
    int n = (int)edges.size();
    vector<int> perm(n);
    perm[start] = 0;
    int idx = 1;
    for (int i = 0; i < n; i++) {
        if (i != start) perm[i] = idx++;
    }
    vector<vector<pair<int,int>>> out(n);
    for (int i = 0; i < n; i++) {
        for (auto& e : edges[i]) {
            out[perm[i]].push_back({perm[e.first], e.second});
        }
    }
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
