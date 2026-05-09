/**
 * Problem 1: Treasure Packing
 * 2D bounded knapsack: 12 categories, each with q_i copies of value v_i,
 * mass m_i, volume l_i. Capacity: 20kg = 2e7 mg, 25L = 2.5e7 µL.
 * Maximize total value within both constraints.
 *
 * Strategy:
 *   - Multiple greedy starts using different weighted ratios
 *     value / (alpha*mass + (1-alpha)*volume) for many alpha values.
 *   - Local search: pair swaps (increase i, decrease j) and pure additions.
 *   - Random restart loop until time budget consumed.
 */

#include <bits/stdc++.h>
#include <chrono>
using namespace std;
using ll = long long;
using namespace std::chrono;

static int n;
static vector<string> names;
static vector<ll> qs, vs_, ms_, ls_;
static const ll Mcap = 20000000LL;
static const ll Lcap = 25000000LL;

static steady_clock::time_point T0;
static double elapsed() {
    return duration_cast<microseconds>(steady_clock::now() - T0).count() / 1e6;
}
static const double TIME_LIMIT = 0.90;

static inline pair<ll,ll> usage(const vector<ll>& x) {
    ll mu = 0, lu = 0;
    for (int i = 0; i < n; i++) { mu += x[i] * ms_[i]; lu += x[i] * ls_[i]; }
    return {mu, lu};
}

static inline ll value(const vector<ll>& x) {
    ll val = 0;
    for (int i = 0; i < n; i++) val += x[i] * vs_[i];
    return val;
}

static vector<ll> greedyOrder(const vector<int>& order) {
    vector<ll> x(n, 0);
    ll mr = Mcap, lr = Lcap;
    for (int i : order) {
        ll take = min({qs[i], mr / ms_[i], lr / ls_[i]});
        x[i] = take;
        mr -= take * ms_[i];
        lr -= take * ls_[i];
    }
    return x;
}

static void localSearch(vector<ll>& x) {
    auto [mu, lu] = usage(x);
    // Repeat until no improvement
    bool changed = true;
    int rounds = 0;
    while (changed && rounds < 200 && elapsed() < TIME_LIMIT) {
        changed = false;
        rounds++;
        // Pure addition
        for (int i = 0; i < n; i++) {
            if (x[i] >= qs[i]) continue;
            ll cap = qs[i] - x[i];
            if (ms_[i] > 0) cap = min(cap, (Mcap - mu) / ms_[i]);
            if (ls_[i] > 0) cap = min(cap, (Lcap - lu) / ls_[i]);
            if (cap > 0) {
                x[i] += cap;
                mu += cap * ms_[i];
                lu += cap * ls_[i];
                changed = true;
            }
        }
        // 1-for-1 swaps
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                if (vs_[i] <= vs_[j]) continue;
                if (x[j] == 0 || x[i] >= qs[i]) continue;
                ll dm = ms_[i] - ms_[j];
                ll dl = ls_[i] - ls_[j];
                if (mu + dm <= Mcap && lu + dl <= Lcap) {
                    x[i]++; x[j]--;
                    mu += dm; lu += dl;
                    changed = true;
                }
            }
        }
        // k-for-r swaps for small (k,r): try pair (i,j) where we add k of i, remove r of j,
        // gain = k*vi - r*vj, must fit. Try with k,r up to small bound.
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                // find best (k, r) exchange that improves
                // Bound k by remaining qs[i] - x[i], bound r by x[j]
                ll maxK = qs[i] - x[i];
                ll maxR = x[j];
                if (maxK == 0 || maxR == 0) continue;
                // Cap to keep search small
                ll Klim = min<ll>(maxK, 128);
                ll Rlim = min<ll>(maxR, 128);
                ll bestK = 0, bestR = 0, bestGain = 0;
                for (ll r = 0; r <= Rlim; r++) {
                    // mass freed = r*ms_[j], vol freed = r*ls_[j]
                    ll availM = Mcap - mu + r * ms_[j];
                    ll availL = Lcap - lu + r * ls_[j];
                    ll kmax = Klim;
                    if (ms_[i] > 0) kmax = min(kmax, availM / ms_[i]);
                    if (ls_[i] > 0) kmax = min(kmax, availL / ls_[i]);
                    if (kmax <= 0) continue;
                    ll gain = kmax * vs_[i] - r * vs_[j];
                    if (gain > bestGain) { bestGain = gain; bestK = kmax; bestR = r; }
                }
                if (bestGain > 0) {
                    x[i] += bestK; x[j] -= bestR;
                    mu += bestK * ms_[i] - bestR * ms_[j];
                    lu += bestK * ls_[i] - bestR * ls_[j];
                    changed = true;
                }
            }
        }
    }
}

int main() {
    T0 = steady_clock::now();
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());

    // Parse JSON: 12 entries, each "name": [q, v, m, l]
    int p = 0, sz = (int)input.size();
    while (p < sz) {
        // Find next quoted name
        while (p < sz && input[p] != '"') p++;
        if (p >= sz) break;
        int sQ = p + 1;
        int eQ = sQ;
        while (eQ < sz && input[eQ] != '"') eQ++;
        string name = input.substr(sQ, eQ - sQ);
        p = eQ + 1;
        // Look for ':' then '[' (this is a key-array entry, not a value-string)
        int q = p;
        while (q < sz && input[q] != ':' && input[q] != '"' && input[q] != '}') q++;
        if (q >= sz || input[q] != ':') continue;
        q++;
        while (q < sz && isspace((unsigned char)input[q])) q++;
        if (q >= sz || input[q] != '[') { p = q; continue; }
        q++;
        ll nums[4] = {0,0,0,0};
        for (int k = 0; k < 4; k++) {
            while (q < sz && !isdigit((unsigned char)input[q]) && input[q] != '-') q++;
            ll sign = 1;
            if (q < sz && input[q] == '-') { sign = -1; q++; }
            ll num = 0;
            while (q < sz && isdigit((unsigned char)input[q])) {
                num = num * 10 + (input[q] - '0'); q++;
            }
            nums[k] = sign * num;
        }
        names.push_back(name);
        qs.push_back(nums[0]);
        vs_.push_back(nums[1]);
        ms_.push_back(nums[2]);
        ls_.push_back(nums[3]);
        p = q;
    }
    n = (int)names.size();

    vector<ll> best(n, 0);
    ll bestVal = 0;

    auto tryX = [&](vector<ll> x) {
        localSearch(x);
        ll v = value(x);
        if (v > bestVal) { bestVal = v; best = x; }
    };

    // Greedy with various weighted ratios
    vector<double> alphas;
    for (int k = 0; k <= 40; k++) alphas.push_back(k / 40.0);
    for (double alpha : alphas) {
        if (elapsed() > TIME_LIMIT * 0.5) break;
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) {
            double da = alpha * ms_[a] + (1.0 - alpha) * ls_[a];
            double db = alpha * ms_[b] + (1.0 - alpha) * ls_[b];
            double ra = (double)vs_[a] / (da + 1e-9);
            double rb = (double)vs_[b] / (db + 1e-9);
            return ra > rb;
        });
        tryX(greedyOrder(idx));
    }

    // Random restarts
    mt19937_64 rng(0xC0FFEE);
    while (elapsed() < TIME_LIMIT) {
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        shuffle(idx.begin(), idx.end(), rng);
        tryX(greedyOrder(idx));
    }

    // Output
    cout << "{\n";
    for (int i = 0; i < n; i++) {
        cout << " \"" << names[i] << "\": " << best[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
