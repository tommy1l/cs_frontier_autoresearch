#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    vector<tuple<long long, int, int>> pieces;

    auto decomposeLen = [&](long long lo, long long hi, int totalBits) {
        long long a = lo;
        while (a <= hi) {
            int j = 0;
            while (true) {
                long long m = (1LL << (j + 1)) - 1;
                if ((a & m) != 0) break;
                if (a + (1LL << (j + 1)) - 1 > hi) break;
                j++;
            }
            int pBits = totalBits - j;
            long long p = a >> j;
            pieces.push_back({p, pBits, j});
            a += (1LL << j);
        }
    };

    for (int k = Kmin; k <= Kmax; k++) {
        long long lo = max(L, 1LL << (k - 1));
        long long hi = min(R, (1LL << k) - 1);
        if (lo <= hi) decomposeLen(lo, hi, k);
    }

    int maxK = 0;
    for (auto& [p, pb, k] : pieces) maxK = max(maxK, k);

    int START = 0;
    auto W = [&](int k) { return 1 + k; };
    int firstTrie = 2 + maxK;
    int nodeCount = firstTrie;

    vector<vector<pair<int,int>>> edges(nodeCount);

    for (int i = 1; i <= maxK; i++) {
        edges[W(i)].push_back({W(i-1), 0});
        edges[W(i)].push_back({W(i-1), 1});
    }

    map<pair<int,int>, int> trieChild;
    // Shared bottom node B(L_bits, k): pieces with the same last 2 prefix bits and trailing-k share this node.
    map<pair<int,int>, int> bottomNode;

    for (auto& [prefix, pBits, k] : pieces) {
        if (pBits == 1) {
            edges[START].push_back({W(k), 1});
            continue;
        }
        if (pBits == 2) {
            int bit_hi = (int)((prefix >> 1) & 1);
            int bit_lo = (int)(prefix & 1);
            auto it = trieChild.find({START, bit_hi});
            int t1;
            if (it == trieChild.end()) {
                t1 = nodeCount++;
                edges.push_back({});
                trieChild[{START, bit_hi}] = t1;
                edges[START].push_back({t1, bit_hi});
            } else {
                t1 = it->second;
            }
            edges[t1].push_back({W(k), bit_lo});
            continue;
        }
        int cur = START;
        for (int d = pBits - 1; d >= 2; d--) {
            int bit = (int)((prefix >> d) & 1);
            auto it = trieChild.find({cur, bit});
            int nxt;
            if (it == trieChild.end()) {
                nxt = nodeCount++;
                edges.push_back({});
                trieChild[{cur, bit}] = nxt;
                edges[cur].push_back({nxt, bit});
            } else {
                nxt = it->second;
            }
            cur = nxt;
        }
        int L_bits = (int)(prefix & 0x3);
        int Lhi = (L_bits >> 1) & 1;
        int Llo = L_bits & 1;
        auto bkey = make_pair(L_bits, k);
        auto bit_it = bottomNode.find(bkey);
        int B;
        if (bit_it == bottomNode.end()) {
            B = nodeCount++;
            edges.push_back({});
            bottomNode[bkey] = B;
            edges[B].push_back({W(k), Llo});
        } else {
            B = bit_it->second;
        }
        edges[cur].push_back({B, Lhi});
    }

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
