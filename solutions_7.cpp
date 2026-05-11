#include <bits/stdc++.h>
using namespace std;

// Iter 11: Simple (B, Lo, Hi) DAFSA, output ordered start=1, end=n (per example convention).

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

    // Find end node (the one with 0 outgoing edges; should be build(0, 0, 0)).
    int endNode = -1;
    auto it = mem.find(make_tuple(0, 0, 0));
    if (it != mem.end()) endNode = it->second;

    int n = (int)edges.size();
    // Permute: start -> 0, end -> n-1, others fill in between.
    vector<int> perm(n);
    perm[start] = 0;
    if (endNode >= 0) perm[endNode] = n - 1;
    int idx = 1;
    for (int i = 0; i < n; i++) {
        if (i == start) continue;
        if (i == endNode) continue;
        perm[i] = idx++;
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
