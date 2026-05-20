// Treasure Packing: 2D bounded knapsack
// Approach: multi-restart hill climbing with random restarts + extensive k-for-m swap neighborhood
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

template<class F>
vector<int> greedy_by(F score) {
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) { return score(a) > score(b); });
    vector<int> take(n, 0);
    long long mass = 0, vol = 0;
    for (int i : idx) {
        long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
        long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
        long long cnt = min({q[i], maxm, maxv});
        if (cnt < 0) cnt = 0;
        take[i] = (int)cnt;
        mass += cnt * mm[i];
        vol  += cnt * ll[i];
    }
    return take;
}

long long compute_val(const vector<int>& take, long long& mass_out, long long& vol_out) {
    long long mass = 0, vol = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        mass += (long long)take[i] * mm[i];
        vol  += (long long)take[i] * ll[i];
        val  += (long long)take[i] * v[i];
    }
    mass_out = mass; vol_out = vol;
    return val;
}

// Deeply polish: try every k-for-m swap up to small k, m
long long deep_polish(vector<int>& take) {
    long long mass, vol, val = compute_val(take, mass, vol);
    bool improved = true;
    while (improved) {
        improved = false;
        // Add one
        for (int i = 0; i < n; ++i) {
            while (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                take[i]++; mass += mm[i]; vol += ll[i]; val += v[i]; improved = true;
            }
        }
        // 1-for-k swap
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (take[i] >= q[i]) continue;
                if (take[j] == 0) continue;
                long long need_m = mass + mm[i] - M_CAP;
                long long need_v = vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_m > 0) k = max(k, (need_m + mm[j] - 1) / mm[j]);
                if (need_v > 0) k = max(k, (need_v + ll[j] - 1) / ll[j]);
                if (k > take[j]) continue;
                long long dval = v[i] - k * v[j];
                if (dval > 0) {
                    take[i]++; take[j] -= (int)k;
                    mass += mm[i] - k * mm[j]; vol += ll[i] - k * ll[j];
                    val += dval; improved = true;
                }
            }
        }
        // k-for-1 swap: add k of i, remove 1 of j
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (take[j] == 0) continue;
                long long free_m = M_CAP - mass + mm[j];
                long long free_v = L_CAP - vol + ll[j];
                long long max_k_m = mm[i] > 0 ? free_m / mm[i] : q[i];
                long long max_k_v = ll[i] > 0 ? free_v / ll[i] : q[i];
                long long max_k = min({(long long)(q[i] - take[i]), max_k_m, max_k_v});
                if (max_k < 2) continue;
                // Find k that maximizes k*v[i] - v[j]
                long long k = max_k;
                long long dval = k * v[i] - v[j];
                if (dval > 0) {
                    take[i] += (int)k; take[j]--;
                    mass += k * mm[i] - mm[j]; vol += k * ll[i] - ll[j];
                    val += dval; improved = true;
                }
            }
        }
    }
    return val;
}

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

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() {
        return chrono::duration<double>(chrono::steady_clock::now() - start_time).count();
    };

    vector<int> best;
    long long best_val = -1;

    // Multi-restart with random initial sorts
    mt19937_64 rng(0xDEADBEEF);
    int restart = 0;
    while (elapsed() < 0.7) {
        vector<int> cur;
        if (restart == 0) {
            cur = greedy_by([&](int i) {
                return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
            });
        } else if (restart == 1) {
            cur = greedy_by([&](int i) { return (double)v[i] / mm[i]; });
        } else if (restart == 2) {
            cur = greedy_by([&](int i) { return (double)v[i] / ll[i]; });
        } else {
            double w = (double)(rng() % 1000) / 1000.0;
            cur = greedy_by([&, w](int i) {
                double r = w * mm[i] / (double)M_CAP + (1.0 - w) * ll[i] / (double)L_CAP;
                if (r < 1e-12) return 1e18;
                return (double)v[i] / r;
            });
        }
        long long val = deep_polish(cur);
        if (val > best_val) { best_val = val; best = cur; }
        restart++;
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
