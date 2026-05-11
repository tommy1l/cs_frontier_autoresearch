#include <bits/stdc++.h>
using namespace std;

// Iter 7: minimum nodes via min-DFA with combined-length states.
// State = a function f: {strings} -> bool. Two states equivalent iff same function.
// We use representation: each state is set of (length, value-range) tuples.
//
// Top-level state: accepts any binary rep of [L,R].
//   Lengths bL..bR; at each length B, value range is [max(L, 2^(B-1)), min(R, 2^B - 1)].
// After first bit "1": new state with each tuple shifted (B -> B-1, range -> range - 2^(B-1)).
// Etc.

struct Rep {
    // (length, Lo, Hi) tuples sorted by length
    vector<tuple<int,int,int>> v;
    bool operator<(const Rep& o) const { return v < o.v; }
    bool operator==(const Rep& o) const { return v == o.v; }
};

map<Rep, int> mem;
vector<vector<pair<int,int>>> edges;

int build(const Rep& r) {
    auto it = mem.find(r);
    if (it != mem.end()) return it->second;
    int sid = (int)edges.size();
    mem[r] = sid;
    edges.push_back({});
    // Compute transitions for bit 0 and bit 1.
    for (int b = 0; b < 2; b++) {
        Rep next;
        for (auto& [B, Lo, Hi] : r.v) {
            if (B == 0) continue; // length-0 cannot have outgoing bit
            int half = 1 << (B - 1);
            int loB, hiB;
            if (b == 0) {
                loB = Lo;
                hiB = min(Hi, half - 1);
            } else {
                loB = max(Lo, half) - half;
                hiB = Hi - half;
            }
            if (loB <= hiB && loB >= 0 && hiB <= half - 1) {
                next.v.push_back({B - 1, loB, hiB});
            }
        }
        if (!next.v.empty()) {
            int child = build(next);
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

    // After first bit "1": for each length B in [bL, bR]:
    //   value range at length B is [max(L, 2^(B-1)), min(R, 2^B - 1)].
    //   After consuming the leading "1", remaining (B-1) bits have value
    //   in [max(L, 2^(B-1)) - 2^(B-1), min(R, 2^B-1) - 2^(B-1)]
    //                = [max(L - 2^(B-1), 0), min(R - 2^(B-1), 2^(B-1) - 1)].
    Rep afterOne;
    for (int B = bL; B <= bR; B++) {
        int half = 1 << (B - 1);
        int lo = max(L, half) - half;
        int hi = min(R, (1 << B) - 1) - half;
        if (lo <= hi) afterOne.v.push_back({B - 1, lo, hi});
    }
    int child = build(afterOne);

    int start = (int)edges.size();
    edges.push_back({});
    edges[start].push_back({child, 1});

    // Renumber so start = 0.
    int n = (int)edges.size();
    vector<int> perm(n);
    perm[start] = 0;
    int idx = 1;
    for (int i = 0; i < n; i++) if (i != start) perm[i] = idx++;
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
