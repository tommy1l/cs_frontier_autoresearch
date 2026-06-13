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

// Persistent column boundary: colb[j] = max i with a[i][j] <= v_cur, 0 if a[1][j] > v_cur.
static int colb[2010];
static long long v_cur;
static int v_cur_set = 0;

static void initSaddleback(long long v) {
    for (int j = 1; j <= N; j++) colb[j] = 0;
    int i = N, j = 1;
    while (i >= 1 && j <= N) {
        long long a = query(i, j);
        if (a <= v) {
            colb[j] = i;
            j++;
        } else {
            i--;
        }
    }
    v_cur = v;
    v_cur_set = 1;
}

static long long countLE(long long v) {
    if (!v_cur_set) {
        initSaddleback(v);
    } else if (v > v_cur) {
        for (int j = 1; j <= N; j++) {
            while (colb[j] < N) {
                long long a = query(colb[j] + 1, j);
                if (a <= v) colb[j]++;
                else break;
            }
        }
        v_cur = v;
    } else if (v < v_cur) {
        for (int j = 1; j <= N; j++) {
            while (colb[j] >= 1) {
                long long a = query(colb[j], j);
                if (a > v) colb[j]--;
                else break;
            }
        }
        v_cur = v;
    }
    long long cnt = 0;
    for (int j = 1; j <= N; j++) cnt += colb[j];
    return cnt;
}

// Heap from (1,1) for kk-th smallest. Push rule: from (x,y) push (x,y+1) always; (x+1,y) only if y==1.
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

    if (minK <= 20000) {
        if (kFromTop <= kFromBot) ans = heapSelectMin(kFromTop);
        else                      ans = heapSelectMax(kFromBot);
    } else {
        // Tighten initial bisect range via global min/max corners.
        long long v00 = query(1, 1);
        long long vNN = query(N, N);
        long long lo = v00 - 1, hi = vNN;
        while (lo + 1 < hi) {
            long long mid = lo + (hi - lo) / 2;
            long long cnt = countLE(mid);
            if (cnt >= K) hi = mid;
            else lo = mid;
        }
        ans = hi;
    }

    printf("DONE %lld\n", ans);
    fflush(stdout);
    return 0;
}
