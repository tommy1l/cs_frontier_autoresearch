// Treasure Packing: 2D bounded knapsack
// Approach: For each pair (i, j) of item categories, enumerate (a, b) quantities at a grid;
// for each (a, b), fill the remaining 10 categories greedily by value-density. Track best.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);

    string s((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    vector<string> names;
    int pos = 0;
    while ((int)names.size() < 12) {
        while (pos < (int)s.size() && s[pos] != '"') pos++;
        if (pos >= (int)s.size()) break;
        int start = ++pos;
        while (pos < (int)s.size() && s[pos] != '"') pos++;
        if (pos >= (int)s.size()) break;
        string name = s.substr(start, pos - start);
        pos++;
        while (pos < (int)s.size() && s[pos] != '[') pos++;
        if (pos >= (int)s.size()) break;
        pos++;
        long long nums[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; ++i) {
            while (pos < (int)s.size() && !isdigit((unsigned char)s[pos])) pos++;
            long long val = 0;
            while (pos < (int)s.size() && isdigit((unsigned char)s[pos])) {
                val = val * 10 + (s[pos] - '0');
                pos++;
            }
            nums[i] = val;
        }
        names.push_back(name);
        q.push_back(nums[0]);
        v.push_back(nums[1]);
        mm.push_back(nums[2]);
        ll.push_back(nums[3]);
        while (pos < (int)s.size() && s[pos] != ']') pos++;
        if (pos < (int)s.size()) pos++;
    }
    n = (int)names.size();

    // For each pair (i, j), precompute a sort order of other items by density
    auto density = [&](int i) {
        return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
    };
    vector<int> all_idx(n);
    iota(all_idx.begin(), all_idx.end(), 0);
    sort(all_idx.begin(), all_idx.end(), [&](int a, int b) { return density(a) > density(b); });

    vector<int> best(n, 0);
    long long best_val = -1;

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    const int GRID = 25;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (elapsed() > 0.7) break;
            for (int ga = 0; ga <= GRID; ++ga) {
                long long a = (q[i] * ga) / GRID;
                for (int gb = 0; gb <= GRID; ++gb) {
                    long long b = (q[j] * gb) / GRID;
                    long long ma = a * mm[i] + b * mm[j];
                    long long va = a * ll[i] + b * ll[j];
                    if (ma > M_CAP || va > L_CAP) continue;
                    vector<int> take(n, 0);
                    take[i] = (int)a; take[j] = (int)b;
                    long long mass = ma, vol = va;
                    long long val = a * v[i] + b * v[j];
                    for (int k : all_idx) {
                        if (k == i || k == j) continue;
                        long long maxm = mm[k] > 0 ? (M_CAP - mass) / mm[k] : q[k];
                        long long maxv = ll[k] > 0 ? (L_CAP - vol)  / ll[k] : q[k];
                        long long cnt = min({q[k], maxm, maxv});
                        if (cnt < 0) cnt = 0;
                        take[k] = (int)cnt;
                        mass += cnt * mm[k];
                        vol  += cnt * ll[k];
                        val  += cnt * v[k];
                    }
                    if (val > best_val) { best_val = val; best = take; }
                }
            }
        }
        if (elapsed() > 0.7) break;
    }

    // Polish
    {
        long long mass = 0, vol = 0;
        for (int i = 0; i < n; ++i) { mass += (long long)best[i] * mm[i]; vol += (long long)best[i] * ll[i]; }
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n; ++i) {
                while (best[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    best[i]++; mass += mm[i]; vol += ll[i]; improved = true;
                }
            }
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
                if (i == j || best[i] >= q[i] || best[j] == 0) continue;
                long long need_m = mass + mm[i] - M_CAP;
                long long need_v = vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_m > 0) k = max(k, (need_m + mm[j] - 1) / mm[j]);
                if (need_v > 0) k = max(k, (need_v + ll[j] - 1) / ll[j]);
                if (k > best[j]) continue;
                long long dval = v[i] - k * v[j];
                if (dval > 0) {
                    best[i]++; best[j] -= (int)k;
                    mass += mm[i] - k * mm[j]; vol += ll[i] - k * ll[j];
                    improved = true;
                }
            }
        }
    }

    cout << "{\n";
    for (int i = 0; i < n; ++i) {
        cout << " \"" << names[i] << "\": " << best[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
