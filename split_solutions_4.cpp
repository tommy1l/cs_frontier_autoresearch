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
    long long a11 = query(1, 1);
    long long ann = query(n, n);
    if (k == 1) {
        cout << "DONE " << a11 << "\n";
        cout.flush();
        return 0;
    }
    if (k == (long long)n * n) {
        cout << "DONE " << ann << "\n";
        cout.flush();
        return 0;
    }
    
    vector<long long> diag(n + 1, 0);
    diag[1] = a11;
    diag[n] = ann;
    for (int i = 2; i <= n - 1; i++) {
        diag[i] = query(i, i);
    }
    
    int lo_i = 1, hi_i = n;
    while (lo_i < hi_i) {
        int mid_i = (lo_i + hi_i) / 2;
        if (countLE(diag[mid_i]) >= k) hi_i = mid_i;
        else lo_i = mid_i + 1;
    }
    
    long long vlo = (lo_i > 1) ? diag[lo_i - 1] + 1 : a11;
    long long vhi = diag[lo_i];
    
    if (vlo >= vhi) {
        cout << "DONE " << vhi << "\n";
        cout.flush();
        return 0;
    }
    
    while (vlo < vhi) {
        long long mid = vlo + (vhi - vlo) / 2;
        if (countLE(mid) >= k) vhi = mid;
        else vlo = mid + 1;
    }
    cout << "DONE " << vlo << "\n";
    cout.flush();
    return 0;
}