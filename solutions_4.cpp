/**
 * Problem 4: Matrix k-th smallest (interactive).
 * Matrix has saddle property (sorted rows + columns).
 * Strategy: heap (Young-tableau k-th) for k <= 50000, binary search on value
 * with cache for larger k.
 */
#include <bits/stdc++.h>
using namespace std;

static int n;
static long long k_target;
static int query_count = 0;
static unordered_map<long long, long long> cache;

static long long key(int x, int y) {
    return (long long)x * 4000LL + (long long)y;
}

static long long query_cell(int x, int y) {
    long long kk = key(x, y);
    auto it = cache.find(kk);
    if (it != cache.end()) return it->second;
    cout << "QUERY " << x << ' ' << y << '\n';
    cout.flush();
    long long v; cin >> v;
    cache[kk] = v;
    query_count++;
    return v;
}

static long long count_le(long long v) {
    // Count cells with a[i][j] <= v using saddle from (1, n).
    long long cnt = 0;
    int i = 1, j = n;
    while (i <= n && j >= 1) {
        long long val = query_cell(i, j);
        if (val <= v) {
            cnt += j;
            i++;
        } else {
            j--;
        }
    }
    return cnt;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n >> k_target;

    if (n == 1) {
        long long v = query_cell(1, 1);
        cout << "DONE " << v << '\n';
        cout.flush();
        return 0;
    }

    if (k_target <= 50000) {
        // Heap-based Young tableau extraction.
        priority_queue<tuple<long long, int, int>,
                       vector<tuple<long long, int, int>>,
                       greater<>> heap;
        unordered_set<long long> seen;
        long long v0 = query_cell(1, 1);
        heap.push({v0, 1, 1});
        seen.insert(key(1, 1));
        long long ans = v0;
        for (long long i = 0; i < k_target; i++) {
            auto [val, x, y] = heap.top();
            heap.pop();
            ans = val;
            if (x + 1 <= n) {
                long long kk = key(x + 1, y);
                if (!seen.count(kk)) {
                    seen.insert(kk);
                    heap.push({query_cell(x + 1, y), x + 1, y});
                }
            }
            if (y + 1 <= n) {
                long long kk = key(x, y + 1);
                if (!seen.count(kk)) {
                    seen.insert(kk);
                    heap.push({query_cell(x, y + 1), x, y + 1});
                }
            }
        }
        cout << "DONE " << ans << '\n';
        cout.flush();
        return 0;
    }

    // Binary search on value.
    long long lo = query_cell(1, 1);
    long long hi = query_cell(n, n);
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        long long c = count_le(mid);
        if (c >= k_target) hi = mid;
        else lo = mid + 1;
    }
    cout << "DONE " << lo << '\n';
    cout.flush();
    return 0;
}
