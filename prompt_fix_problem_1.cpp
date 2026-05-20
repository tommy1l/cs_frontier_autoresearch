// Treasure Packing: 2D bounded knapsack
// Approach: Branch and bound. Order items by value-density. At each level branch on quantity.
// Use LP upper bound = min(fractional-mass-knapsack, fractional-vol-knapsack) over remaining items.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

static vector<int> ord;
static vector<int> cur_take, best_take;
static long long best_val;
static chrono::steady_clock::time_point start_time;
static bool budget_exceeded = false;

double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - start_time).count();
}

double lp_bound(int from, long long mass_left, long long vol_left) {
    double bm = 0.0, bv = 0.0;
    for (int t = from; t < n; ++t) {
        int i = ord[t];
        long long maxm = mm[i] > 0 ? mass_left / mm[i] : q[i];
        long long take_m = min((long long)q[i], maxm);
        bm += (double)take_m * v[i];
        long long rem_m = mass_left - take_m * mm[i];
        if (take_m < q[i] && mm[i] > 0 && rem_m > 0) {
            bm += (double)v[i] * rem_m / (double)mm[i];
        }
        long long maxv = ll[i] > 0 ? vol_left / ll[i] : q[i];
        long long take_v = min((long long)q[i], maxv);
        bv += (double)take_v * v[i];
        long long rem_v = vol_left - take_v * ll[i];
        if (take_v < q[i] && ll[i] > 0 && rem_v > 0) {
            bv += (double)v[i] * rem_v / (double)ll[i];
        }
    }
    return min(bm, bv);
}

void dfs(int depth, long long cur_val, long long mass_used, long long vol_used) {
    if (budget_exceeded) return;
    if (depth == n) {
        if (cur_val > best_val) { best_val = cur_val; best_take = cur_take; }
        return;
    }
    if (elapsed() > 0.65) { budget_exceeded = true; return; }
    // Bound
    long long mass_left = M_CAP - mass_used;
    long long vol_left  = L_CAP - vol_used;
    double bound = cur_val + lp_bound(depth, mass_left, vol_left);
    if (bound <= (double)best_val + 0.5) return;
    int i = ord[depth];
    long long maxm = mm[i] > 0 ? mass_left / mm[i] : q[i];
    long long maxv = ll[i] > 0 ? vol_left  / ll[i] : q[i];
    long long maxk = min({q[i], maxm, maxv});
    if (maxk < 0) maxk = 0;
    // Try quantities from maxk down to 0
    for (long long k = maxk; k >= 0; --k) {
        cur_take[i] = (int)k;
        dfs(depth + 1, cur_val + k * v[i], mass_used + k * mm[i], vol_used + k * ll[i]);
        if (budget_exceeded) return;
    }
    cur_take[i] = 0;
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

    // Initialize incumbent with greedy
    ord.assign(n, 0);
    iota(ord.begin(), ord.end(), 0);
    auto density = [&](int i) {
        return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
    };
    sort(ord.begin(), ord.end(), [&](int a, int b) { return density(a) > density(b); });

    cur_take.assign(n, 0);
    best_take.assign(n, 0);
    best_val = 0;
    {
        long long mass = 0, vol = 0;
        for (int i : ord) {
            long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
            long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
            long long cnt = min({q[i], maxm, maxv});
            if (cnt < 0) cnt = 0;
            best_take[i] = (int)cnt;
            mass += cnt * mm[i];
            vol  += cnt * ll[i];
            best_val += cnt * v[i];
        }
    }

    start_time = chrono::steady_clock::now();
    dfs(0, 0, 0, 0);

    cout << "{\n";
    for (int i = 0; i < n; ++i) {
        cout << " \"" << names[i] << "\": " << best_take[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
