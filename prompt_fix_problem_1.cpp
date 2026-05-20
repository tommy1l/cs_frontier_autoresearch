// Treasure Packing: 2D bounded knapsack
// Approach: Large Neighborhood Search. Repeatedly "destroy" a random subset of categories
// (set them to 0), then "repair" by re-running greedy on those categories with the leftover capacity.
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

    auto density = [&](int i) {
        return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
    };
    vector<int> order_global(n);
    iota(order_global.begin(), order_global.end(), 0);
    sort(order_global.begin(), order_global.end(), [&](int a, int b) { return density(a) > density(b); });

    auto greedy_full = [&]() {
        vector<int> take(n, 0);
        long long mass = 0, vol = 0;
        for (int i : order_global) {
            long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
            long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
            long long cnt = min({q[i], maxm, maxv});
            if (cnt < 0) cnt = 0;
            take[i] = (int)cnt;
            mass += cnt * mm[i];
            vol  += cnt * ll[i];
        }
        return take;
    };

    vector<int> best = greedy_full();
    long long best_val = 0;
    for (int i = 0; i < n; ++i) best_val += (long long)best[i] * v[i];

    mt19937_64 rng(0xFADECAFE);
    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    while (elapsed() < 0.7) {
        // Choose destroy size 2..6
        int dsize = 2 + (int)(rng() % 5);
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        shuffle(idx.begin(), idx.end(), rng);
        vector<int> dest(idx.begin(), idx.begin() + dsize);
        vector<int> trial = best;
        long long mass = 0, vol = 0;
        for (int d : dest) trial[d] = 0;
        for (int i = 0; i < n; ++i) { mass += (long long)trial[i] * mm[i]; vol += (long long)trial[i] * ll[i]; }
        // Repair: greedy on destroyed items by random density-based order
        vector<int> rebuild_order(dest);
        // Sometimes shuffle, sometimes sort by density to bias diversification
        if (rng() & 1) shuffle(rebuild_order.begin(), rebuild_order.end(), rng);
        else sort(rebuild_order.begin(), rebuild_order.end(), [&](int a, int b) { return density(a) > density(b); });
        for (int i : rebuild_order) {
            long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
            long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
            long long cnt = min({q[i], maxm, maxv});
            if (cnt < 0) cnt = 0;
            trial[i] = (int)cnt;
            mass += cnt * mm[i];
            vol  += cnt * ll[i];
        }
        // Local polish
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n; ++i) {
                while (trial[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    trial[i]++; mass += mm[i]; vol += ll[i]; improved = true;
                }
            }
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
                if (i == j || trial[i] >= q[i] || trial[j] == 0) continue;
                long long need_m = mass + mm[i] - M_CAP;
                long long need_v = vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_m > 0) k = max(k, (need_m + mm[j] - 1) / mm[j]);
                if (need_v > 0) k = max(k, (need_v + ll[j] - 1) / ll[j]);
                if (k > trial[j]) continue;
                long long dval = v[i] - k * v[j];
                if (dval > 0) {
                    trial[i]++; trial[j] -= (int)k;
                    mass += mm[i] - k * mm[j]; vol += ll[i] - k * ll[j];
                    improved = true;
                }
            }
        }
        long long val = 0;
        for (int i = 0; i < n; ++i) val += (long long)trial[i] * v[i];
        if (val > best_val) { best_val = val; best = trial; }
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
