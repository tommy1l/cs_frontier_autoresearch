// Treasure Packing: 2D bounded knapsack
// Approach: Iterated Local Search. Run deterministic local search to local optimum,
// then perturb with a random kick (large randomized swap), then re-optimize.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

long long compute(const vector<int>& t, long long& mass_out, long long& vol_out) {
    long long mass = 0, vol = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        mass += (long long)t[i] * mm[i];
        vol  += (long long)t[i] * ll[i];
        val  += (long long)t[i] * v[i];
    }
    mass_out = mass; vol_out = vol;
    return val;
}

void local_search(vector<int>& take, long long& mass, long long& vol, long long& val) {
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n; ++i) {
            while (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                take[i]++; mass += mm[i]; vol += ll[i]; val += v[i]; improved = true;
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
                val += dval; improved = true;
            }
        }
    }
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

    // Initial: greedy by density
    auto greedy = [&]() {
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) {
            return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) >
                   (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
        });
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
    };

    vector<int> cur = greedy();
    long long mass, vol;
    long long val = compute(cur, mass, vol);
    local_search(cur, mass, vol, val);

    vector<int> best = cur;
    long long best_val = val;

    mt19937_64 rng(0xABCDEF12);
    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    while (elapsed() < 0.7) {
        // Kick: perturb 2-4 random items by removing some quantity (down to 0)
        vector<int> kicked = best;
        long long km, kv;
        long long kval = compute(kicked, km, kv);
        int kicks = 2 + (int)(rng() % 3);
        for (int kk = 0; kk < kicks; ++kk) {
            int i = (int)(rng() % n);
            int target = (int)(rng() % (q[i] + 1));
            int delta = target - kicked[i];
            kicked[i] = target;
            km += (long long)delta * mm[i];
            kv += (long long)delta * ll[i];
            kval += (long long)delta * v[i];
        }
        // Repair if needed
        while (km > M_CAP || kv > L_CAP) {
            int worst = -1; double worst_score = 1e18;
            for (int i = 0; i < n; ++i) {
                if (kicked[i] == 0) continue;
                double sc = (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
                if (sc < worst_score) { worst_score = sc; worst = i; }
            }
            if (worst < 0) break;
            kicked[worst]--;
            km -= mm[worst]; kv -= ll[worst]; kval -= v[worst];
        }
        local_search(kicked, km, kv, kval);
        if (kval > best_val) {
            best_val = kval;
            best = kicked;
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
