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

// Count cells with a[i][j] <= v using staircase from (N, 1).
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

int main() {
    if (scanf("%d %lld", &N, &K) != 2) return 0;
    long long lo = 0, hi = (long long)2e18;
    // Smallest v with countLE(v) >= K is the K-th smallest value.
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (countLE(mid) >= K) hi = mid;
        else lo = mid + 1;
    }
    printf("DONE %lld\n", lo);
    fflush(stdout);
    return 0;
}
