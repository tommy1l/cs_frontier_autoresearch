#include <bits/stdc++.h>
using namespace std;

int n;
long long k;
unordered_map<long long, long long> cache;

long long query(int i, int j) {
    long long key = (long long)i * 200001LL + j;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    cout << "QUERY " << i << " " << j << "\n";
    cout.flush();
    long long v;
    cin >> v;
    cache[key] = v;
    return v;
}

long long countLE(long long v) {
    long long cnt = 0;
    int i = 1, j = n;
    while (i <= n && j >= 1) {
        if (cnt >= k) return cnt;
        long long upperBound = cnt + (long long)(n - i + 1) * j;
        if (upperBound < k) return cnt;
        long long x = query(i, j);
        if (x <= v) {
            cnt += j;
            i++;
        } else {
            j--;
        }
    }
    return cnt;
}

int main() {
    cin >> n >> k;
    if (n == 1) {
        long long v = query(1, 1);
        cout << "DONE " << v << "\n";
        cout.flush();
        return 0;
    }
    long long lo = query(1, 1);
    long long hi = query(n, n);
    if (k == 1) {
        cout << "DONE " << lo << "\n";
        cout.flush();
        return 0;
    }
    if (k == (long long)n * n) {
        cout << "DONE " << hi << "\n";
        cout.flush();
        return 0;
    }
    
    vector<long long> anchors = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, (long long)n};
    
    // Pass A: tighten hi
    for (long long i : anchors) {
        if (i < 1 || i > n) continue;
        long long jH = (k + i - 1) / i;
        if (jH < 1 || jH > n) continue;
        long long v = query((int)i, (int)jH);
        hi = min(hi, v);
    }
    
    // Pass B: tighten lo
    for (long long i : anchors) {
        if (i < 1 || i > n) continue;
        long long m = (long long)n - i + 1;
        long long target = (long long)n * n - k + 1;
        long long p = (target + m - 1) / m;
        if (p < 1 || p > n) continue;
        long long jL = (long long)n + 1 - p;
        if (jL < 1 || jL > n) continue;
        long long v = query((int)i, (int)jL);
        lo = max(lo, v);
    }
    
    if (lo == hi) {
        cout << "DONE " << lo << "\n";
        cout.flush();
        return 0;
    }
    
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (countLE(mid) >= k) hi = mid;
        else lo = mid + 1;
    }
    cout << "DONE " << lo << "\n";
    cout.flush();
    return 0;
}