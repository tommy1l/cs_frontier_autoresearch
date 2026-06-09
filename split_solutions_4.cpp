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
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (countLE(mid) >= k) hi = mid;
        else lo = mid + 1;
    }
    cout << "DONE " << lo << "\n";
    cout.flush();
    return 0;
}