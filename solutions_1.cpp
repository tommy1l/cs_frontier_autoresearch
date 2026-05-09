/**
 * Problem 1: Treasure Packing
 * 2D bounded knapsack: 12 categories, q_i copies of value v_i,
 * mass m_i, volume l_i. Capacity 20kg / 25L.
 *
 * Strategy:
 *   - Heuristic phase: multi-greedy by mixed density + local search
 *     (pair swaps, k-for-r exchanges).
 *   - Exact phase: branch-and-bound on item counts, LP upper bound
 *     = min(LP-relax-by-mass, LP-relax-by-volume). Items sorted by
 *     v/m density so the natural traversal gives a tight bound.
 */

#include <bits/stdc++.h>
#include <chrono>
using namespace std;
using ll = long long;
using namespace std::chrono;

static int n;
static vector<string> names;
static vector<ll> qs, vs_, ms_, ls_;
static vector<int> volOrder;
static const ll Mcap = 20000000LL;
static const ll Lcap = 25000000LL;

static steady_clock::time_point T0;
static double elapsed() {
    return duration_cast<microseconds>(steady_clock::now() - T0).count() / 1e6;
}
static const double TIME_LIMIT = 0.92;
static const double HEUR_BUDGET = 0.35;

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
    bool changed = true;
    int rounds = 0;
    while (changed && rounds < 200 && elapsed() < TIME_LIMIT) {
        changed = false;
        rounds++;
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
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                ll maxK = qs[i] - x[i];
                ll maxR = x[j];
                if (maxK == 0 || maxR == 0) continue;
                ll Klim = min<ll>(maxK, 128);
                ll Rlim = min<ll>(maxR, 128);
                ll bestK = 0, bestR = 0, bestGain = 0;
                for (ll r = 0; r <= Rlim; r++) {
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
        // 1-for-2 swap: +1 of i, -1 of j, -1 of k
        for (int i = 0; i < n; i++) {
            if (x[i] >= qs[i]) continue;
            for (int j = 0; j < n; j++) {
                if (j == i || x[j] == 0) continue;
                for (int k = j + 1; k < n; k++) {
                    if (k == i || x[k] == 0) continue;
                    ll gain = vs_[i] - vs_[j] - vs_[k];
                    if (gain <= 0) continue;
                    ll dm = ms_[i] - ms_[j] - ms_[k];
                    ll dl = ls_[i] - ls_[j] - ls_[k];
                    if (mu + dm <= Mcap && lu + dl <= Lcap) {
                        x[i]++; x[j]--; x[k]--;
                        mu += dm; lu += dl;
                        changed = true;
                    }
                }
            }
        }
        // 2-for-1 swap: +1 of i, +1 of j, -1 of k
        for (int i = 0; i < n; i++) {
            if (x[i] >= qs[i]) continue;
            for (int j = i + 1; j < n; j++) {
                if (x[j] >= qs[j]) continue;
                for (int k = 0; k < n; k++) {
                    if (k == i || k == j || x[k] == 0) continue;
                    ll gain = vs_[i] + vs_[j] - vs_[k];
                    if (gain <= 0) continue;
                    ll dm = ms_[i] + ms_[j] - ms_[k];
                    ll dl = ls_[i] + ls_[j] - ls_[k];
                    if (mu + dm <= Mcap && lu + dl <= Lcap) {
                        x[i]++; x[j]++; x[k]--;
                        mu += dm; lu += dl;
                        changed = true;
                    }
                }
            }
        }
    }
}

static ll bestVal;
static vector<ll> best_x, cur_x;

static double upperBound(int idx, ll mLeft, ll lLeft) {
    // LP relaxation by mass alone, items idx..n-1 in v/m order.
    double ub_m = 0;
    ll mr = mLeft;
    for (int i = idx; i < n; i++) {
        if (mr <= 0) break;
        ll t = min(qs[i], mr / ms_[i]);
        ub_m += (double)t * vs_[i];
        mr -= t * ms_[i];
        if (mr > 0 && t < qs[i]) {
            ub_m += (double)mr * vs_[i] / (double)ms_[i];
            mr = 0;
            break;
        }
    }
    // LP relaxation by volume alone, items in v/l order, skip those already decided.
    double ub_l = 0;
    ll lr = lLeft;
    for (int i : volOrder) {
        if (i < idx) continue;
        if (lr <= 0) break;
        ll t = min(qs[i], lr / ls_[i]);
        ub_l += (double)t * vs_[i];
        lr -= t * ls_[i];
        if (lr > 0 && t < qs[i]) {
            ub_l += (double)lr * vs_[i] / (double)ls_[i];
            lr = 0;
            break;
        }
    }
    return min(ub_m, ub_l);
}

static int timeCheckCounter = 0;
static bool timeUp = false;

static void bnb(int idx, ll mLeft, ll lLeft, ll val) {
    if (val > bestVal) { bestVal = val; best_x = cur_x; }
    if (idx == n) return;
    if ((++timeCheckCounter & 1023) == 0) {
        if (elapsed() > TIME_LIMIT) { timeUp = true; }
    }
    if (timeUp) return;
    double ub = (double)val + upperBound(idx, mLeft, lLeft);
    if (ub < (double)bestVal + 1.0 - 1e-6) return;
    ll maxTake = qs[idx];
    if (ms_[idx] > 0) maxTake = min(maxTake, mLeft / ms_[idx]);
    if (ls_[idx] > 0) maxTake = min(maxTake, lLeft / ls_[idx]);
    for (ll t = maxTake; t >= 0; t--) {
        cur_x[idx] = t;
        bnb(idx + 1, mLeft - t * ms_[idx], lLeft - t * ls_[idx], val + t * vs_[idx]);
        if (timeUp) break;
    }
    cur_x[idx] = 0;
}

int main() {
    T0 = steady_clock::now();
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());

    int p = 0, sz = (int)input.size();
    while (p < sz) {
        while (p < sz && input[p] != '"') p++;
        if (p >= sz) break;
        int sQ = p + 1;
        int eQ = sQ;
        while (eQ < sz && input[eQ] != '"') eQ++;
        string name = input.substr(sQ, eQ - sQ);
        p = eQ + 1;
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

    // Reorder all arrays by v/m desc — gives tight LP-by-mass UB during B&B.
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return (long double)vs_[a] * ms_[b] > (long double)vs_[b] * ms_[a];
        });
        vector<string> nN(n); vector<ll> nQ(n), nV(n), nM(n), nL(n);
        for (int k = 0; k < n; k++) {
            int o = ord[k];
            nN[k] = names[o]; nQ[k] = qs[o]; nV[k] = vs_[o]; nM[k] = ms_[o]; nL[k] = ls_[o];
        }
        names = nN; qs = nQ; vs_ = nV; ms_ = nM; ls_ = nL;
    }

    volOrder.resize(n);
    iota(volOrder.begin(), volOrder.end(), 0);
    sort(volOrder.begin(), volOrder.end(), [&](int a, int b) {
        return (long double)vs_[a] * ls_[b] > (long double)vs_[b] * ls_[a];
    });

    // Heuristic: seed B&B with a strong lower bound.
    bestVal = 0;
    best_x.assign(n, 0);

    auto tryX = [&](vector<ll> x) {
        localSearch(x);
        ll v = value(x);
        if (v > bestVal) { bestVal = v; best_x = x; }
    };

    vector<double> alphas;
    for (int k = 0; k <= 40; k++) alphas.push_back(k / 40.0);
    for (double alpha : alphas) {
        if (elapsed() > HEUR_BUDGET * 0.6) break;
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
    mt19937_64 rng(0xC0FFEE);
    while (elapsed() < HEUR_BUDGET) {
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        shuffle(idx.begin(), idx.end(), rng);
        tryX(greedyOrder(idx));
    }

    // Branch and bound on item counts, in v/m order, max-first.
    cur_x.assign(n, 0);
    bnb(0, Mcap, Lcap, 0);

    cout << "{\n";
    for (int i = 0; i < n; i++) {
        cout << " \"" << names[i] << "\": " << best_x[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
