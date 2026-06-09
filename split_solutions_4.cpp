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
    
    // Diagonal precomputation
    vector<long long> diag(n + 1, 0);
    diag[1] = lo;
    diag[n] = hi;
    for (int i = 2; i <= n - 1; i++) {
        diag[i] = query(i, i);
    }
    
    // Find i_hi: smallest i in [1,n] with i*i >= k
    long long target_hi = k;
    int i_hi = -1;
    for (int i = 1; i <= n; i++) {
        if ((long long)i * i >= target_hi) {
            i_hi = i;
            break;
        }
    }
    if (i_hi != -1) {
        hi = diag[i_hi];
    }
    
    // Find i_lo: largest i in [1,n] with (n-i+1)*(n-i+1) >= n*n - k + 1
    long long target_lo = (long long)n * n - k + 1;
    int i_lo = -1;
    for (int i = n; i >= 1; i--) {
        long long m = n - i + 1;
        if (m * m >= target_lo) {
            i_lo = i;
            break;
        }
    }
    if (i_lo != -1) {
        lo = diag[i_lo];
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