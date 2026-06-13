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
        // diag-anchor + analytic-bounds: sample sqrt(N) main-diagonal cells,
        // classify each via submatrix LB/UB on count(<=v_i), skip countLE on
        // samples already resolved analytically.
        int S = max(2, (int)sqrt((double)N) + 1);
        vector<long long> samp(S);
        vector<int> idxv(S);
        for (int t = 0; t < S; t++) {
            idxv[t] = 1 + (int)((long long)t * (N - 1) / (S - 1));
            samp[t] = query(idxv[t], idxv[t]);
        }
        // samp[] is non-decreasing (main diagonal monotone). LB(t)=idxv[t]^2,
        // UB(t)=M-(N-idxv[t]+1)^2+1. Both monotone in t.
        // analyt_lo = last t with UB(t) < K (v_t strictly below answer).
        // analyt_hi = first t with LB(t) >= K (v_t at-or-above answer).
        int analyt_lo = -1, analyt_hi = S;
        for (int t = 0; t < S; t++) {
            long long ubcount = M - (long long)(N - idxv[t] + 1) * (N - idxv[t] + 1) + 1;
            if (ubcount < K) analyt_lo = t;
        }
        for (int t = S - 1; t >= 0; t--) {
            long long lbcount = (long long)idxv[t] * idxv[t];
            if (lbcount >= K) analyt_hi = t;
        }
        long long Lbound = (analyt_lo >= 0) ? samp[analyt_lo] + 1 : 1;
        long long Hbound = (analyt_hi < S) ? samp[analyt_hi] : (long long)2e18;
        // Binary search within unknown band [analyt_lo+1 .. analyt_hi-1] using countLE.
        int ilo_s = analyt_lo;
        int ihi_s = analyt_hi;
        while (ihi_s - ilo_s > 1) {
            int mid = (ilo_s + ihi_s) / 2;
            long long c = countLE(samp[mid], K);
            if (c >= K) {
                ihi_s = mid;
                if (samp[mid] < Hbound) Hbound = samp[mid];
            } else {
                ilo_s = mid;
                if (samp[mid] + 1 > Lbound) Lbound = samp[mid] + 1;
            }
        }
        if (Lbound > Hbound) Lbound = Hbound;
        // Final value bisect in tightened range, with early-exit countLE.
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
