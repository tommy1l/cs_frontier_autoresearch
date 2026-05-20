// Treasure Packing: 2D bounded knapsack
// Approach: GRASP. Each iteration: build a solution by randomized greedy (pick from top-k candidates each step),
// then run a deterministic local search to a local optimum. Keep best across restarts.
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

    mt19937_64 rng(0x12345678);
    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    vector<int> best(n, 0);
    long long best_val = -1;

    while (elapsed() < 0.7) {
        vector<int> take(n, 0);
        long long mass = 0, vol = 0;
        vector<int> remaining(n);
        iota(remaining.begin(), remaining.end(), 0);
        // Randomized greedy: at each step pick from top-k candidates by density
        while (!remaining.empty()) {
            sort(remaining.begin(), remaining.end(), [&](int a, int b) { return density(a) > density(b); });
            int top_k = min(3, (int)remaining.size());
            int chosen = (int)(rng() % top_k);
            int i = remaining[chosen];
            long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
            long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
            long long cnt = min({q[i], maxm, maxv});
            if (cnt < 0) cnt = 0;
            take[i] = (int)cnt;
            mass += cnt * mm[i];
            vol  += cnt * ll[i];
            remaining.erase(remaining.begin() + chosen);
        }
        // Local search polish
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n; ++i) {
                while (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    take[i]++; mass += mm[i]; vol += ll[i]; improved = true;
                }
            }
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
                if (i == j || take[i] >= q[i] || take[j] == 0) continue;
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
                    improved = true;
                }
            }
        }
        long long val = 0;
        for (int i = 0; i < n; ++i) val += (long long)take[i] * v[i];
        if (val > best_val) { best_val = val; best = take; }
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
