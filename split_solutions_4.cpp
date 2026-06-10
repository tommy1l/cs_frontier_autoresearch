#include <bits/stdc++.h>
using namespace std;

int n;
long long k;
unordered_map<long long, long long> cache;
vector<pair<int, long long>> col1Vals, colNVals;
vector<pair<int, long long>> row1Vals, rowNVals;
long long lo, hi;

long long query(int i, int j) {
    long long key = (long long)i * 200001LL + j;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    cout << "QUERY " << i << " " << j << "\n";
    cout.flush();
    long long v;
    cin >> v;
    cache[key] = v;
    if ((long long)i * (long long)j >= k) hi = min(hi, v);
    long long m = (long long)(n - i + 1) * (long long)(n - j + 1);
    if (m >= (long long)n * (long long)n - k + 1) lo = max(lo, v);
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
    int jSkipN = 0;
    for (int idx = (int)rowNVals.size() - 1; idx >= 0; idx--) {
        if (rowNVals[idx].second <= v) {
            jSkipN = rowNVals[idx].first;
            break;
        }
    }
    int jSkipZero = n + 1;
    for (int idx = 0; idx < (int)row1Vals.size(); idx++) {
        if (row1Vals[idx].second > v) {
            jSkipZero = row1Vals[idx].first;
            break;
        }
    }
    int iLow = iSkipN, iHigh = iSkipZero - 1;
    int jLow = jSkipN, jHigh = jSkipZero - 1;
    long long cnt = (long long)iLow * n + (long long)(n - iLow) * jLow;
    if (cnt >= k) return cnt;
    if (iLow + 1 > iHigh || jLow + 1 > jHigh) return cnt;
    int i = iLow + 1, j = jHigh;
    while (i <= iHigh && j > jLow) {
        if (cnt >= k) return cnt;
        long long upperBound = cnt + (long long)(iHigh - i + 1) * (j - jLow);
        if (upperBound < k) return cnt;
        long long x = query(i, j);
        if (x <= v) {
            cnt += (j - jLow);
            i++;
        } else {
            j--;
        }
    }
    return cnt;
}

int main() {
    cin >> n >> k;
    lo = LLONG_MIN;
    hi = LLONG_MAX;
    if (n == 1) {
        long long v = query(1, 1);
        cout << "DONE " << v << "\n";
        cout.flush();
        return 0;
    }
    long long a11 = query(1, 1);
    long long ann = query(n, n);
    lo = max(lo, a11);
    hi = min(hi, ann);
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
    
    long long target_hi = k;
    int i_hi = -1;
    for (int i = 1; i <= n; i++) {
        if ((long long)i * i >= target_hi) {
            i_hi = i;
            break;
        }
    }
    if (i_hi != -1) {
        long long v = query(i_hi, i_hi);
        hi = min(hi, v);
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
        long long v = query(i_lo, i_lo);
        lo = max(lo, v);
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
    
    long long jListArr[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, (long long)n};
    set<int> usedJ;
    for (long long jj : jListArr) {
        if (jj < 1 || jj > n) continue;
        int j = (int)jj;
        if (usedJ.count(j)) continue;
        usedJ.insert(j);
        long long v1 = query(1, j);
        row1Vals.push_back({j, v1});
        long long vN = query(n, j);
        rowNVals.push_back({j, vN});
    }
    sort(row1Vals.begin(), row1Vals.end());
    sort(rowNVals.begin(), rowNVals.end());
    
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
    for (auto& p : rowNVals) {
        if ((long long)p.first * n >= k) {
            hi = min(hi, p.second);
        }
    }
    for (auto& p : row1Vals) {
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
        if (countLE(mid) >= k) hi = min(hi, mid);
        else lo = max(lo, mid + 1);
    }
    cout << "DONE " << lo << "\n";
    cout.flush();
    return 0;
}