#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    int START = 0;
    vector<vector<pair<int,int>>> edges;
    edges.push_back({});
    int END = (int)edges.size();
    edges.push_back({});

    vector<int> wNode(Kmax + 2, -1);
    wNode[0] = END;

    function<int(int)> getW = [&](int j) -> int {
        if (wNode[j] != -1) return wNode[j];
        int id = (int)edges.size();
        edges.push_back({});
        wNode[j] = id;
        int prev = getW(j-1);
        edges[id].push_back({prev, 0});
        edges[id].push_back({prev, 1});
        return id;
    };

    auto getBit = [&](long long v, int pos) -> int { return (int)((v >> pos) & 1); };

    function<void(int, long long, int)> buildLower = [&](int from, long long val, int nbits) {
        int cur = from;
        for (int i = 0; i < nbits; i++) {
            int rem = nbits - 1 - i;
            int b = getBit(val, rem);
            if (b == 0) {
                if (rem == 0) {
                    edges[cur].push_back({END, 0});
                    edges[cur].push_back({END, 1});
                } else {
                    int next = (int)edges.size();
                    edges.push_back({});
                    edges[cur].push_back({next, 0});
                    edges[cur].push_back({getW(rem), 1});
                    cur = next;
                }
            } else {
                if (rem == 0) {
                    edges[cur].push_back({END, 1});
                } else {
                    int next = (int)edges.size();
                    edges.push_back({});
                    edges[cur].push_back({next, 1});
                    cur = next;
                }
            }
        }
    };

    function<void(int, long long, int)> buildUpper = [&](int from, long long val, int nbits) {
        int cur = from;
        for (int i = 0; i < nbits; i++) {
            int rem = nbits - 1 - i;
            int b = getBit(val, rem);
            if (b == 1) {
                if (rem == 0) {
                    edges[cur].push_back({END, 0});
                    edges[cur].push_back({END, 1});
                } else {
                    int next = (int)edges.size();
                    edges.push_back({});
                    edges[cur].push_back({getW(rem), 0});
                    edges[cur].push_back({next, 1});
                    cur = next;
                }
            } else {
                if (rem == 0) {
                    edges[cur].push_back({END, 0});
                } else {
                    int next = (int)edges.size();
                    edges.push_back({});
                    edges[cur].push_back({next, 0});
                    cur = next;
                }
            }
        }
    };

    for (int k = Kmin; k <= Kmax; k++) {
        long long Lk = max(L, 1LL << (k - 1));
        long long Rk = min(R, (1LL << k) - 1);
        if (Lk > Rk) continue;

        if (Lk == (1LL << (k - 1)) && Rk == (1LL << k) - 1) {
            if (k == 1) {
                edges[START].push_back({END, 1});
            } else {
                edges[START].push_back({getW(k - 1), 1});
            }
            continue;
        }

        if (Lk == Rk) {
            int cur = START;
            for (int i = 0; i < k; i++) {
                int rem = k - 1 - i;
                int b = getBit(Lk, rem);
                if (rem == 0) {
                    edges[cur].push_back({END, b});
                } else {
                    int next = (int)edges.size();
                    edges.push_back({});
                    edges[cur].push_back({next, b});
                    cur = next;
                }
            }
            continue;
        }

        int c = 0;
        while (c < k && getBit(Lk, k-1-c) == getBit(Rk, k-1-c)) c++;

        int cur = START;
        for (int i = 0; i < c; i++) {
            int b = getBit(Lk, k-1-i);
            int next = (int)edges.size();
            edges.push_back({});
            edges[cur].push_back({next, b});
            cur = next;
        }
        int rem = k - c;
        long long Lk_suf = Lk & ((1LL << (rem - 1)) - 1);
        long long Rk_suf = Rk & ((1LL << (rem - 1)) - 1);

        if (rem - 1 == 0) {
            edges[cur].push_back({END, 0});
        } else if (Lk_suf == 0) {
            edges[cur].push_back({getW(rem - 1), 0});
        } else {
            int low_start = (int)edges.size();
            edges.push_back({});
            edges[cur].push_back({low_start, 0});
            buildLower(low_start, Lk_suf, rem - 1);
        }

        if (rem - 1 == 0) {
            edges[cur].push_back({END, 1});
        } else if (Rk_suf == (1LL << (rem - 1)) - 1) {
            edges[cur].push_back({getW(rem - 1), 1});
        } else {
            int up_start = (int)edges.size();
            edges.push_back({});
            edges[cur].push_back({up_start, 1});
            buildUpper(up_start, Rk_suf, rem - 1);
        }
    }

    int nodeCount = (int)edges.size();
    cout << nodeCount << "\n";
    for (int i = 0; i < nodeCount; i++) {
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) {
            cout << " " << (a + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
