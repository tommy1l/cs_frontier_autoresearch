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
        // level-set regime: sample on two non-diagonal analytic curves bracketing a_K.
        // ABOVE curve: (i, j) with i*j >= K, so LB analytic >= K => a[i][j] >= a_K.
        //   j_above(i) = ceil(K/i), valid when j_above <= N (i.e., i >= ceil(K/N)).
        // BELOW curve: (i, j) with (N-i+1)*(N-j+1) > M-K, so UB analytic < K => a[i][j] <= a_K.
        //   j_below(i) = N - floor((M-K)/(N-i+1)), valid when j_below >= 1.
        // Each above-sample is an analytic upper bound; each below-sample a lower bound.
        // Final bisect uses tight Lbound/Hbound from the sandwich.
        int S = max(4, (int)cbrt((double)N) + 1);
        long long Lbound = 1, Hbound = (long long)2e18;

        // ABOVE curve sampling: i in [i_lo, N].
        long long i_above_lo = (K + N - 1) / N;
        if (i_above_lo < 1) i_above_lo = 1;
        long long i_above_hi = N;
        if (i_above_lo <= i_above_hi) {
            long long range_a = i_above_hi - i_above_lo;
            long long cnt_a = min((long long)S, range_a + 1);
            for (long long t = 0; t < cnt_a; t++) {
                long long i;
                if (cnt_a == 1) i = i_above_lo;
                else i = i_above_lo + t * range_a / (cnt_a - 1);
                long long j = (K + i - 1) / i;
                if (j < 1) j = 1;
                if (j > N) continue;
                long long v = query((int)i, (int)j);
                if (v < Hbound) Hbound = v;
            }
        }

        // BELOW curve sampling: i in [1, i_hi].
        long long denom = N - 1;
        long long ceil_div = (denom > 0) ? ((M - K) + denom - 1) / denom : (M - K);
        long long i_below_hi = N + 1 - ceil_div;
        long long i_below_lo = 1;
        if (i_below_hi > N) i_below_hi = N;
        if (i_below_lo <= i_below_hi) {
            long long range_b = i_below_hi - i_below_lo;
            long long cnt_b = min((long long)S, range_b + 1);
            for (long long t = 0; t < cnt_b; t++) {
                long long i;
                if (cnt_b == 1) i = i_below_lo;
                else i = i_below_lo + t * range_b / (cnt_b - 1);
                long long div = N - i + 1;
                long long j = N - (M - K) / div;
                if (j < 1) continue;
                if (j > N) j = N;
                long long v = query((int)i, (int)j);
                if (v + 1 > Lbound) Lbound = v + 1;
            }
        }

        if (Lbound > Hbound) Lbound = Hbound;
        // Final value bisect with early-exit countLE.
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
