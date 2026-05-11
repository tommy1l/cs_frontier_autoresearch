/**
 * Problem 4: Matrix k-th smallest (interactive).
 * Strategy:
 *   - small k (≤ n): Young-tableau heap from top-left.
 *   - large k (≥ n*n - n): Young-tableau heap from bottom-right (find rank
 *     n*n - k + 1 of the largest).
 *   - medium k via row-merge from whichever side is closer.
 *   - very far middle (no closer side): fall back to binary search on value.
 */
#include <bits/stdc++.h>
using namespace std;

static int n;
static long long k_target;
static unordered_map<long long, long long> cache;

static long long key(int x, int y) {
    return (long long)x * 4010LL + (long long)y;
}

static long long query_cell(int x, int y) {
    long long kk = key(x, y);
    auto it = cache.find(kk);
    if (it != cache.end()) return it->second;
    cout << "QUERY " << x << ' ' << y << '\n';
    cout.flush();
    long long v; cin >> v;
    cache[kk] = v;
    return v;
}

static long long young_from_topleft(long long k) {
    priority_queue<tuple<long long, int, int>,
                   vector<tuple<long long, int, int>>,
                   greater<>> heap;
    unordered_set<long long> seen;
    heap.push({query_cell(1, 1), 1, 1});
    seen.insert(key(1, 1));
    long long ans = 0;
    for (long long i = 0; i < k; i++) {
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
    return ans;
}

static long long young_from_bottomright(long long k) {
    // Pop k-th LARGEST from bottom-right.
    priority_queue<tuple<long long, int, int>> heap; // max-heap
    unordered_set<long long> seen;
    heap.push({query_cell(n, n), n, n});
    seen.insert(key(n, n));
    long long ans = 0;
    for (long long i = 0; i < k; i++) {
        auto [val, x, y] = heap.top();
        heap.pop();
        ans = val;
        if (x - 1 >= 1) {
            long long kk = key(x - 1, y);
            if (!seen.count(kk)) {
                seen.insert(kk);
                heap.push({query_cell(x - 1, y), x - 1, y});
            }
        }
        if (y - 1 >= 1) {
            long long kk = key(x, y - 1);
            if (!seen.count(kk)) {
                seen.insert(kk);
                heap.push({query_cell(x, y - 1), x, y - 1});
            }
        }
    }
    return ans;
}

static long long row_merge_from_start(long long k) {
    // Find k-th smallest. Init: first cell of each row.
    priority_queue<tuple<long long, int, int>,
                   vector<tuple<long long, int, int>>,
                   greater<>> pq;
    for (int r = 1; r <= n; r++) {
        pq.push({query_cell(r, 1), r, 1});
    }
    long long ans = 0;
    for (long long i = 0; i < k; i++) {
        auto [val, r, c] = pq.top();
        pq.pop();
        ans = val;
        if (c + 1 <= n) {
            pq.push({query_cell(r, c + 1), r, c + 1});
        }
    }
    return ans;
}

static long long row_merge_from_end(long long k) {
    // Find k-th smallest from end: pop (n*n - k + 1)-th largest.
    long long pops = (long long)n * n - k + 1;
    priority_queue<tuple<long long, int, int>> pq; // max-heap
    for (int r = 1; r <= n; r++) {
        pq.push({query_cell(r, n), r, n});
    }
    long long ans = 0;
    for (long long i = 0; i < pops; i++) {
        auto [val, r, c] = pq.top();
        pq.pop();
        ans = val;
        if (c - 1 >= 1) {
            pq.push({query_cell(r, c - 1), r, c - 1});
        }
    }
    return ans;
}

static long long count_le(long long v) {
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

static long long binary_search_value() {
    // Sample the main diagonal to tighten [lo, hi].
    vector<long long> diag(n + 2, 0);
    for (int i = 1; i <= n; i++) diag[i] = query_cell(i, i);
    long long lo = diag[1];
    long long hi = diag[n];
    int i_lo = 1, i_hi = n;
    for (int i = 1; i <= n; i++) {
        long long lt_max = (long long)(i - 1) * (2LL * n - i + 1);
        if (lt_max < k_target && diag[i] >= lo) {
            lo = diag[i];
            i_lo = i;
        }
        long long le_min = (long long)i * i;
        if (le_min >= k_target) {
            if (diag[i] <= hi) { hi = diag[i]; i_hi = i; }
            break;
        }
    }
    // Initialize counts from analytical rank bounds:
    //   diag[i] has lt-rank ≤ (i-1)(2n-i+1) and le-rank ≥ i*i.
    // Use midpoint as estimate; clamp to maintain c_lo < k <= c_hi.
    auto avg_rank = [&](int i) -> long long {
        long long lt_max = (long long)(i - 1) * (2LL * n - i + 1);
        long long le_min = (long long)i * i;
        return (lt_max + le_min) / 2;
    };
    long long c_lo = avg_rank(i_lo);
    long long c_hi = avg_rank(i_hi);
    if (c_lo >= k_target) c_lo = k_target - 1;
    if (c_hi < k_target) c_hi = k_target;

    while (lo < hi) {
        long long mid;
        if (c_hi > c_lo) {
            __int128 span = c_hi - c_lo;
            __int128 target = (long long)k_target - c_lo;
            __int128 offset = (__int128)(hi - lo) * target / span;
            mid = lo + (long long)offset;
        } else {
            mid = lo + (hi - lo) / 2;
        }
        if (mid < lo) mid = lo;
        if (mid >= hi) mid = hi - 1;
        long long c = count_le(mid);
        if (c >= k_target) { hi = mid; c_hi = c; }
        else { lo = mid + 1; c_lo = c; }
    }
    return lo;
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

    long long nn = (long long)n * n;
    long long ans;

    long long from_start = k_target;
    long long from_end = nn - k_target + 1;

    if (k_target <= n) {
        ans = young_from_topleft(k_target);
    } else if (from_end <= n) {
        ans = young_from_bottomright(from_end);
    } else if (from_start <= 48000 - n) {
        ans = row_merge_from_start(k_target);
    } else if (from_end <= 48000 - n) {
        ans = row_merge_from_end(k_target);
    } else {
        ans = binary_search_value();
    }

    cout << "DONE " << ans << '\n';
    cout.flush();
    return 0;
}
