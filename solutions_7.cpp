#include <bits/stdc++.h>
using namespace std;

// Iter 4: explicit L/R/free chain construction with long long arithmetic.

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int B = 0;
    for (long long x = R; x; x >>= 1) B++;

    map<tuple<int, long long, long long>, int> mem;
    vector<vector<pair<int, int>>> edges;

    function<int(int, long long, long long)> build = [&](int Bw, long long Lo, long long Hi) -> int {
        auto key = make_tuple(Bw, Lo, Hi);
        auto it = mem.find(key);
        if (it != mem.end()) return it->second;
        int sid = (int)edges.size();
        mem[key] = sid;
        edges.push_back({});
        if (Bw == 0) return sid;
        long long mid = 1LL << (Bw - 1);
        long long Lo0 = Lo, Hi0 = min(Hi, mid - 1);
        if (Lo0 <= Hi0) {
            int child = build(Bw - 1, Lo0, Hi0);
            edges[sid].push_back({child, 0});
        }
        if (Hi >= mid) {
            long long Lo1 = max(Lo, mid) - mid;
            long long Hi1 = Hi - mid;
            int child = build(Bw - 1, Lo1, Hi1);
            edges[sid].push_back({child, 1});
        }
        return sid;
    };

    int start = (int)edges.size();
    edges.push_back({});

    long long top_lo = L - (1LL << (B - 1));
    long long top_hi = R - (1LL << (B - 1));
    int child = build(B - 1, top_lo, top_hi);
    edges[start].push_back({child, 1});

    int n = (int)edges.size();
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << edges[i].size();
        for (auto& e : edges[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
