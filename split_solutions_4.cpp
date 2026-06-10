#include <bits/stdc++.h>
using namespace std;

int n;
long long k;
unordered_map<long long, long long> cache;
vector<pair<int, long long>> col1Vals, colNVals;

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
    int iSkipN = 0;
    for (int idx = (int)colNVals.size() - 1; idx >= 0; idx--) {
        if (colNVals[idx].second <= v) {
            iSkipN = colNVals[idx].first;
            break;
        }
    }
    int iSkipZero = n + 1;
    for (int idx = 0; idx < (int)col1Vals.size(); idx++) {
        if (col1Vals[idx].second > v) {
            iSkipZero = col1Vals[idx].first;
            break;
        }
    }
    long long cnt = (long long)n * iSkipN;
    if (cnt >= k) return cnt;
    int i = iSkipN + 1, j = n;
    int iMax = iSkipZero - 1;
    if (i > iMax) return cnt;
    while (i <= iMax && j >= 1) {
        if (cnt >= k) return cnt;
        long long upperBound = cnt + (long long)(iMax - i + 1) * j;
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
    
    long long target_hi = k;
    int i_hi = -1;
    for (int i = 1; i <= n; i++) {
        if ((long long)i * i >= target_hi) {
            i_hi = i;
            break;
        }
    }
    if (i_hi != -1) {
        hi = query(i_hi, i_hi);
    }
    
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
        lo = query(i_lo, i_lo);
    }
    
    long long iListArr[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, (long long)n};
    set<int> usedI;
    for (long long ii : iListArr) {
        if (ii < 1 || ii > n) continue;
        int i = (int)ii;
        if (usedI.count(i)) continue;
        usedI.insert(i);
        long long v1 = query(i, 1);
        col1Vals.push_back({i, v1});
        long long vN = query(i, n);
        colNVals.push_back({i, vN});
    }
    sort(col1Vals.begin(), col1Vals.end());
    sort(colNVals.begin(), colNVals.end());
    
    for (auto& p : colNVals) {
        if ((long long)p.first * n >= k) {
            hi = min(hi, p.second);
        }
    }
    for (auto& p : col1Vals) {
        if ((long long)(p.first - 1) * n < k) {
            lo = max(lo, p.second);
        }
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