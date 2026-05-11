#include <bits/stdc++.h>
using namespace std;

// Iter 14: Build DFA by considering each "endpoint pattern" explicitly.
// State key = (mode, depth, value).  Mode: 0=free, 1=tight-lo, 2=tight-hi, 3=tight-both.

struct Key {
    int mode, depth, vlo, vhi;
    bool operator<(const Key& o) const {
        return tie(mode, depth, vlo, vhi) < tie(o.mode, o.depth, o.vlo, o.vhi);
    }
};

map<Key, int> mem;
vector<vector<pair<int,int>>> edges;
int endNode;

int build(int mode, int depth, int vlo, int vhi);

int build(int mode, int depth, int vlo, int vhi) {
    Key k = {mode, depth, vlo, vhi};
    auto it = mem.find(k);
    if (it != mem.end()) return it->second;
    int sid = (int)edges.size();
    mem[k] = sid;
    edges.push_back({});

    if (depth == 0) return sid;

    int mid = 1 << (depth - 1);

    int lo, hi;
    if (mode == 0) { lo = 0; hi = mid * 2 - 1; }
    else if (mode == 1) { lo = vlo; hi = mid * 2 - 1; }
    else if (mode == 2) { lo = 0; hi = vhi; }
    else { lo = vlo; hi = vhi; }

    // bit 0 path
    int lo0 = lo, hi0 = min(hi, mid - 1);
    if (lo0 <= hi0) {
        // new bounds at depth-1: [lo0, hi0]
        int nlo = lo0, nhi = hi0;
        int nmode;
        if (nlo == 0 && nhi == mid - 1) nmode = 0;
        else if (nlo == 0) nmode = 2;
        else if (nhi == mid - 1) nmode = 1;
        else nmode = 3;
        int child = build(nmode, depth - 1, nlo, nhi);
        edges[sid].push_back({child, 0});
    }
    // bit 1 path
    if (hi >= mid) {
        int lo1 = max(lo, mid) - mid, hi1 = hi - mid;
        int nlo = lo1, nhi = hi1;
        int nmode;
        if (nlo == 0 && nhi == mid - 1) nmode = 0;
        else if (nlo == 0) nmode = 2;
        else if (nhi == mid - 1) nmode = 1;
        else nmode = 3;
        int child = build(nmode, depth - 1, nlo, nhi);
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

    endNode = build(0, 0, 0, 0);  // creates depth-0 node (end)
    int start = (int)edges.size();
    edges.push_back({});

    auto pickMode = [&](int depth, int lo, int hi) {
        int top = (1 << depth) - 1;
        if (lo == 0 && hi == top) return 0;
        if (lo == 0) return 2;
        if (hi == top) return 1;
        return 3;
    };

    if (bL == bR) {
        int B = bL, mid = 1 << (B - 1);
        int lo = L - mid, hi = R - mid;
        int child = build(pickMode(B - 1, lo, hi), B - 1, lo, hi);
        edges[start].push_back({child, 1});
    } else {
        int midL = 1 << (bL - 1);
        int childL = build(pickMode(bL - 1, L - midL, midL - 1), bL - 1, L - midL, midL - 1);
        edges[start].push_back({childL, 1});
        for (int B = bL + 1; B < bR; B++) {
            int childM = build(0, B - 1, 0, (1 << (B - 1)) - 1);
            edges[start].push_back({childM, 1});
        }
        int midR = 1 << (bR - 1);
        int childR = build(pickMode(bR - 1, 0, R - midR), bR - 1, 0, R - midR);
        edges[start].push_back({childR, 1});
    }

    int n = (int)edges.size();
    vector<int> perm(n);
    perm[start] = 0;
    int idx = 1;
    for (int i = 0; i < n; i++) if (i != start) perm[i] = idx++;
    vector<vector<pair<int,int>>> out(n);
    for (int i = 0; i < n; i++)
        for (auto& e : edges[i])
            out[perm[i]].push_back({perm[e.first], e.second});

    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) cout << " " << (e.first + 1) << " " << e.second;
        cout << "\n";
    }
    return 0;
}
