#include <bits/stdc++.h>
using namespace std;

static int N;
static long long K;
static unordered_map<long long, long long> cache;

static inline long long ckey(int x, int y) {
    return (long long)x * 2050LL + (long long)y;
}

static long long query(int x, int y) {
    long long k = ckey(x, y);
    auto it = cache.find(k);
    if (it != cache.end()) return it->second;
    printf("QUERY %d %d\n", x, y);
    fflush(stdout);
    long long v;
    if (scanf("%lld", &v) != 1) v = 0;
    cache[k] = v;
    return v;
}

// Saddleback countLE from (N, 1) with early-exit at K_target.
// Returned value is only safe to compare against K_target.
static long long countLE(long long v, long long K_target) {
    long long cnt = 0;
    int i = N, j = 1;
    while (i >= 1 && j <= N) {
        long long a = query(i, j);
        if (a <= v) {
            cnt += (long long)i;
            if (cnt >= K_target) return cnt;
            j++;
        } else {
            i--;
        }
    }
    return cnt;
}

static long long heapSelectMin(long long kk) {
    using P = tuple<long long, int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    long long v00 = query(1, 1);
    pq.push({v00, 1, 1});
    long long popped = 0;
    long long last = 0;
    while (!pq.empty()) {
        auto [v, x, y] = pq.top(); pq.pop();
        popped++;
        last = v;
        if (popped == kk) return last;
        if (y + 1 <= N) pq.push({query(x, y + 1), x, y + 1});
        if (y == 1 && x + 1 <= N) pq.push({query(x + 1, y), x + 1, y});
    }
    return last;
}

static long long heapSelectMax(long long kk) {
    using P = tuple<long long, int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    long long vNN = query(N, N);
    pq.push({-vNN, N, N});
    long long popped = 0;
    long long last = 0;
    while (!pq.empty()) {
        auto [nv, x, y] = pq.top(); pq.pop();
        popped++;
        last = -nv;
        if (popped == kk) return last;
        if (y - 1 >= 1) pq.push({-query(x, y - 1), x, y - 1});
        if (y == N && x - 1 >= 1) pq.push({-query(x - 1, y), x - 1, y});
    }
    return last;
}

int main() {
    if (scanf("%d %lld", &N, &K) != 2) return 0;
    long long M = (long long)N * N;
    long long kFromTop = K;
    long long kFromBot = M - K + 1;
    long long minK = min(kFromTop, kFromBot);
    long long ans = 0;

    if (minK <= 20000) {
        if (kFromTop <= kFromBot) ans = heapSelectMin(kFromTop);
        else                      ans = heapSelectMax(kFromBot);
    } else {
        // diag-anchor: sample sqrt(N) main-diagonal cells (monotone in i),
        // sandwich K among them, then bisect inside the tightened range.
        int S = max(2, (int)sqrt((double)N) + 1);
        vector<long long> samp;
        samp.reserve(S);
        for (int t = 0; t < S; t++) {
            int idx = 1 + (int)((long long)t * (N - 1) / (S - 1));
            samp.push_back(query(idx, idx));
        }
        sort(samp.begin(), samp.end());
        samp.erase(unique(samp.begin(), samp.end()), samp.end());
        // Find tightest pair (ilo, ihi) with countLE(samp[ilo])<K<=countLE(samp[ihi]).
        int ilo_s = 0, ihi_s = (int)samp.size() - 1;
        // Drive ihi_s down: smallest index with countLE >= K.
        while (ihi_s - ilo_s > 1) {
            int mid = (ilo_s + ihi_s) / 2;
            long long c = countLE(samp[mid], K);
            if (c >= K) ihi_s = mid;
            else ilo_s = mid;
        }
        long long Lbound, Hbound;
        // Verify countLE(samp[ilo_s]) status; if it's >= K, then ihi_s
        // really is samp[ilo_s] and Lbound is matrix min.
        long long cLo = countLE(samp[ilo_s], K);
        if (cLo >= K) {
            Hbound = samp[ilo_s];
            // No lower diagonal anchor; fall back to 0.
            Lbound = 0;
        } else {
            Lbound = samp[ilo_s] + 1;
            Hbound = samp[ihi_s];
        }
        if (Lbound > Hbound) Lbound = Hbound;
        // Standard value bisect in tightened range, with early-exit countLE.
        while (Lbound < Hbound) {
            long long mid = Lbound + (Hbound - Lbound) / 2;
            if (countLE(mid, K) >= K) Hbound = mid;
            else Lbound = mid + 1;
        }
        ans = Lbound;
    }
    printf("DONE %lld\n", ans);
    fflush(stdout);
    return 0;
}
