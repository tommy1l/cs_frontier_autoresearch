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
        // Multi-level diag sandwich. Maintain (iLoDiag, iHiDiag) such that
        // countLE(a[iLoDiag,iLoDiag]) < K and countLE(a[iHiDiag,iHiDiag]) >= K.
        // Sentinels: iLoDiag=0 (vLo=0, countLE=0), iHiDiag=N+1 (vHi=2e18, countLE=M).
        int iLoDiag = 0, iHiDiag = N + 1;
        long long vLo = 0;
        long long vHi = (long long)2e18;

        int safeguard = 0;
        while (iHiDiag - iLoDiag > 1 && safeguard < 8) {
            safeguard++;
            int lowIdx = (iLoDiag == 0) ? 1 : iLoDiag + 1;
            int highIdx = (iHiDiag == N + 1) ? N : iHiDiag - 1;
            int interior = highIdx - lowIdx + 1;
            if (interior <= 0) break;
            int subS;
            if (iLoDiag == 0 && iHiDiag == N + 1) {
                subS = max(2, (int)sqrt((double)N) + 1);
            } else {
                subS = max(2, (int)sqrt((double)interior) + 1);
            }
            if (subS > interior) subS = interior;

            vector<pair<long long,int>> samp;
            samp.reserve(subS);
            for (int t = 0; t < subS; t++) {
                int idx;
                if (subS == 1) idx = (lowIdx + highIdx) / 2;
                else idx = lowIdx + (int)((long long)t * (highIdx - lowIdx) / (subS - 1));
                if (idx < lowIdx) idx = lowIdx;
                if (idx > highIdx) idx = highIdx;
                samp.push_back({query(idx, idx), idx});
            }
            // Diagonal values are monotone non-decreasing in idx -> samp sorted by .first.
            // Binary search: loS = largest idx with countLE < K, hiS = smallest with countLE >= K.
            int loS = -1, hiS = (int)samp.size();
            int l = 0, r = (int)samp.size() - 1;
            while (l <= r) {
                int m = (l + r) / 2;
                long long c = countLE(samp[m].first, K);
                if (c < K) { loS = m; l = m + 1; }
                else       { hiS = m; r = m - 1; }
            }
            int newLoDiag = iLoDiag, newHiDiag = iHiDiag;
            long long newVLo = vLo, newVHi = vHi;
            if (loS >= 0) { newLoDiag = samp[loS].second; newVLo = samp[loS].first; }
            if (hiS < (int)samp.size()) { newHiDiag = samp[hiS].second; newVHi = samp[hiS].first; }
            if (newLoDiag == iLoDiag && newHiDiag == iHiDiag) break; // no progress
            iLoDiag = newLoDiag; iHiDiag = newHiDiag;
            vLo = newVLo; vHi = newVHi;
        }

        long long Lbound = (iLoDiag > 0) ? vLo + 1 : 0;
        long long Hbound = (iHiDiag <= N) ? vHi : (long long)2e18;
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
