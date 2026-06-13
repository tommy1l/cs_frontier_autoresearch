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

// Saddleback countLE from (N, 1).
static long long countLE(long long v) {
    long long cnt = 0;
    int i = N, j = 1;
    while (i >= 1 && j <= N) {
        long long a = query(i, j);
        if (a <= v) {
            cnt += (long long)i;
            j++;
        } else {
            i--;
        }
    }
    return cnt;
}

// Heap-based merge from (1,1). Returns kk-th smallest.
// Push rule (each cell pushed exactly once):
//   from popped (x,y): push (x, y+1) always; push (x+1, y) only if y==1.
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
        if (y + 1 <= N) {
            long long vv = query(x, y + 1);
            pq.push({vv, x, y + 1});
        }
        if (y == 1 && x + 1 <= N) {
            long long vv = query(x + 1, y);
            pq.push({vv, x + 1, y});
        }
    }
    return last;
}

// Mirror: heap from (N,N) for kk-th largest.
// Push rule: from popped (x,y): push (x, y-1) always; push (x-1, y) only if y==N.
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
        if (y - 1 >= 1) {
            long long vv = query(x, y - 1);
            pq.push({-vv, x, y - 1});
        }
        if (y == N && x - 1 >= 1) {
            long long vv = query(x - 1, y);
            pq.push({-vv, x - 1, y});
        }
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
    // Heap-corner: queries ~ 2*minK + N. Use when comfortably under budget.
    if (minK <= 20000) {
        if (kFromTop <= kFromBot) ans = heapSelectMin(kFromTop);
        else                      ans = heapSelectMax(kFromBot);
    } else {
        long long lo = 0, hi = (long long)2e18;
        while (lo < hi) {
            long long mid = lo + (hi - lo) / 2;
            if (countLE(mid) >= K) hi = mid;
            else lo = mid + 1;
        }
        ans = lo;
    }
    printf("DONE %lld\n", ans);
    fflush(stdout);
    return 0;
}
