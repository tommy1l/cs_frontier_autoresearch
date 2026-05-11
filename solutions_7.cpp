#include <bits/stdc++.h>
using namespace std;

// Iter 3: NFA-style: start has 2 weight-1 edges, one for L-side (numbers
// with prefix "10...") and one for R-side (numbers with prefix "11...").
// Each side has its own L-track or R-track + shared free chain.

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

    long long L, R;
    cin >> L >> R;

    int bL = 0, bR = 0;
    for (long long x = L; x; x >>= 1) bL++;
    for (long long x = R; x; x >>= 1) bR++;

    int start = (int)edges.size();
    edges.push_back({});

    if (bL == bR) {
        int B = bL;
        long long mid = 1LL << (B - 1);
        // Find the "split" point: smallest k where L's k-th bit (MSB-indexed) differs from R's.
        // Below the split (k < split), L and R agree.
        int split = 1;
        for (int k = 1; k < B; k++) {
            long long mask = 1LL << (B - 1 - k);
            // Actually easier: just check bit B-k from L and R.
            int lb = (L >> (B - k - 1)) & 1; // bit at position k+1 from MSB... hmm
            int rb = (R >> (B - k - 1)) & 1;
            if (lb != rb) { split = k; break; }
        }
        // Hmm this split logic might be off. Let me simplify:
        // Split = number of leading bits where L and R agree.
        split = 0;
        for (int k = B - 1; k >= 0; k--) {
            if (((L >> k) & 1) == ((R >> k) & 1)) split++;
            else break;
        }
        // For our test, L's MSB = R's MSB = 1, but L's 2nd MSB = 0, R's = 1. So split = 1.

        // L-side: numbers with prefix = L's first (split+1) bits and bit at split+1 = 0.
        // Wait, actually L-side covers numbers in [L, L_prefix · 0 · 1^(B-split-1)].
        // = [L, (L's first split bits + "0" + 1^(B-split-1))].
        // Similarly R-side.

        long long L_top_bits = L >> (B - split);
        long long L_side_max = (L_top_bits << (B - split)) | ((1LL << (B - split - 1)) - 1) | (0LL << (B - split - 1));
        // L-side max = L_top_bits + "0" + "1...1" (B-split-1 ones)
        L_side_max = (L_top_bits << (B - split)) + ((1LL << (B - split - 1)) - 1);
        long long R_side_min = (L_top_bits << (B - split)) + (1LL << (B - split - 1));

        // L-side root: (B - split, L - L_top_bits * 2^(B-split), 2^(B-split-1) - 1).
        long long L_sub_lo = L - (L_top_bits << (B - split));
        long long L_sub_hi = (1LL << (B - split - 1)) - 1;
        int childL = build(B - split, L_sub_lo, L_sub_hi);
        edges[start].push_back({childL, 1});

        // R-side root: similar
        long long R_top_bits = R >> (B - split);
        long long R_sub_lo = 1LL << (B - split - 1);
        long long R_sub_hi = R - (R_top_bits << (B - split));
        int childR = build(B - split, R_sub_lo, R_sub_hi);
        edges[start].push_back({childR, 1});

        // Wait: L_top_bits and R_top_bits should be equal (they agree on the first 'split' bits).
        // The first bit of L = first bit of R = 1 (since bL == bR), and start has weight-1 edge.
        // But we have 'split' leading agreement bits. So we'd need split weight-1 edges from start? No.

        // Hmm this is getting confused. Let me just use the simple case: start → (subgraph for [L, R] of 19-bit suffix).
        // The 19-bit suffix is in [L - 2^(B-1), R - 2^(B-1)]. Then split inside that.

        // Reset.
        edges.clear();
        mem.clear();
        start = (int)edges.size();
        edges.push_back({});

        // Top-level: (B-1, L_lo, R_hi) where L_lo = L - 2^(B-1), R_hi = R - 2^(B-1).
        long long lo = L - (1LL << (B - 1));
        long long hi = R - (1LL << (B - 1));
        long long sub_mid = 1LL << (B - 2);

        // Split at depth 2: left side = (B-1)-bit suffix in [lo, sub_mid - 1], right = [sub_mid, hi].
        if (lo <= sub_mid - 1) {
            // L-side
            int leftSide = build(B - 1, lo, sub_mid - 1);
            edges[start].push_back({leftSide, 1});
        }
        if (hi >= sub_mid) {
            int rightSide = build(B - 1, sub_mid, hi);
            edges[start].push_back({rightSide, 1});
        }
    } else {
        // Multi-bit-length not expected for this test
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

    int n = edges.size();
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
