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
        // diag-anchor + DOUBLE sandwich: S main-diag samples at (idxv[t], idxv[t])
        // PLUS S-1 upper sandwich at (idxv[t], idxv[t+1]) AND S-1 lower (anti)
        // sandwich at (idxv[t+1], idxv[t]). Both off-diag samples in each t-pair
        // share the same per-sample LB/UB but typically have distinct values, so:
        //   - smaller off-diag uses tightened UB from union(BR_upper, BR_lower)
        //   - larger off-diag uses tightened LB from union(TL_upper, TL_lower)
        // densifying the sample band AND tightening analytic-skip per pair.
        int S = max(4, (int)cbrt((double)N) + 1);
        vector<long long> diag_v(S), upper_v(S - 1), lower_v(S - 1);
        vector<int> idxv(S);
        for (int t = 0; t < S; t++) {
            idxv[t] = 1 + (int)((long long)t * (N - 1) / (S - 1));
            diag_v[t] = query(idxv[t], idxv[t]);
        }
        for (int t = 0; t < S - 1; t++) {
            upper_v[t] = query(idxv[t], idxv[t + 1]);
            lower_v[t] = query(idxv[t + 1], idxv[t]);
        }

        // Interleaved sorted layout per t: diag_v[t], min(off), max(off), diag_v[t+1], ...
        // Sorted because diag_v[t] <= upper/lower[t] <= diag_v[t+1] (mono rows/cols).
        int TS = 3 * S - 2;
        vector<long long> vals(TS);
        vector<long long> lbcnt(TS), ubcnt(TS);
        int pos = 0;
        for (int t = 0; t < S; t++) {
            vals[pos] = diag_v[t];
            lbcnt[pos] = (long long)idxv[t] * idxv[t];
            ubcnt[pos] = M - (long long)(N - idxv[t] + 1) * (N - idxv[t] + 1) + 1;
            pos++;
            if (t < S - 1) {
                long long it = idxv[t], it1 = idxv[t + 1];
                long long lo = min(upper_v[t], lower_v[t]);
                long long hi = max(upper_v[t], lower_v[t]);
                long long lb_single = it * it1;
                long long ub_single = M - (long long)(N - it + 1) * (N - it1 + 1) + 1;
                long long lb_union = 2 * it * it1 - it * it;
                long long bru = (long long)(N - it + 1) * (N - it1 + 1);
                long long inter = (long long)(N - it1 + 1) * (N - it1 + 1);
                long long ub_union = M - 2 * bru + inter + 1;
                vals[pos] = lo;
                lbcnt[pos] = lb_single;
                ubcnt[pos] = ub_union;
                pos++;
                vals[pos] = hi;
                lbcnt[pos] = lb_union;
                ubcnt[pos] = ub_single;
                pos++;
            }
        }

        int analyt_lo = -1, analyt_hi = TS;
        for (int k = 0; k < TS; k++) {
            if (ubcnt[k] < K) analyt_lo = k;
        }
        for (int k = TS - 1; k >= 0; k--) {
            if (lbcnt[k] >= K) analyt_hi = k;
        }
        long long Lbound = (analyt_lo >= 0) ? vals[analyt_lo] + 1 : 1;
        long long Hbound = (analyt_hi < TS) ? vals[analyt_hi] : (long long)2e18;
        // Binary search within unknown band using countLE.
        int ilo_s = analyt_lo;
        int ihi_s = analyt_hi;
        while (ihi_s - ilo_s > 1) {
            int mid = (ilo_s + ihi_s) / 2;
            long long c = countLE(vals[mid], K);
            if (c >= K) {
                ihi_s = mid;
                if (vals[mid] < Hbound) Hbound = vals[mid];
            } else {
                ilo_s = mid;
                if (vals[mid] + 1 > Lbound) Lbound = vals[mid] + 1;
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
