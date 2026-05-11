/**
 * Problem 1: Treasure Packing — 2D bounded knapsack.
 *
 * Strategy:
 *   - Heuristic seed (multi-greedy + local search) for tight LB.
 *   - Branch & bound on item counts; UB = min(1D LP-by-mass, 1D LP-by-volume,
 *     Lagrangian dual at root). The Lagrangian dual is the true 2D LP
 *     relaxation and is much tighter when both constraints bind at LP opt.
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
static const double TIME_LIMIT = 0.95;
static const double HEUR_BUDGET = 0.92;

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
                ll Klim = min<ll>(maxK, 512);
                ll Rlim = min<ll>(maxR, 512);
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
        // 3-item exchange: drop 1 of a, 1 of b, gain k of i (a != b, both != i).
        for (int i = 0; i < n; i++) {
            if (x[i] >= qs[i]) continue;
            for (int a = 0; a < n; a++) {
                if (a == i || x[a] == 0) continue;
                for (int b = a + 1; b < n; b++) {
                    if (b == i || x[b] == 0) continue;
                    ll availM = Mcap - mu + ms_[a] + ms_[b];
                    ll availL = Lcap - lu + ls_[a] + ls_[b];
                    ll kmax = qs[i] - x[i];
                    if (ms_[i] > 0) kmax = min(kmax, availM / ms_[i]);
                    if (ls_[i] > 0) kmax = min(kmax, availL / ls_[i]);
                    if (kmax <= 0) continue;
                    ll gain = kmax * vs_[i] - vs_[a] - vs_[b];
                    if (gain > 0) {
                        x[i] += kmax; x[a]--; x[b]--;
                        mu += kmax * ms_[i] - ms_[a] - ms_[b];
                        lu += kmax * ls_[i] - ls_[a] - ls_[b];
                        changed = true;
                    }
                }
            }
        }
    }
}

// Lagrangian dual: min over (lm, ll_) >= 0 of
//   lm*M + ll_*L + sum_i q_i max(0, v_i - lm m_i - ll_ l_i)
// This is the true 2D LP relaxation upper bound.
static double lambda_m_root = 0, lambda_l_root = 0;

static double optLambdaMfull(double ll_) {
    vector<pair<double,double>> bp;
    double S = 0;
    for (int i = 0; i < n; i++) {
        double r0 = (double)vs_[i] - ll_ * (double)ls_[i];
        if (r0 > 0) {
            bp.push_back({r0 / (double)ms_[i], (double)qs[i] * (double)ms_[i]});
            S += (double)qs[i] * (double)ms_[i];
        }
    }
    if (S <= (double)Mcap) return 0;
    sort(bp.begin(), bp.end());
    for (auto& p : bp) {
        S -= p.second;
        if (S <= (double)Mcap) return p.first;
    }
    return bp.empty() ? 0 : bp.back().first;
}

static double optLambdaLfull(double lm) {
    vector<pair<double,double>> bp;
    double S = 0;
    for (int i = 0; i < n; i++) {
        double r0 = (double)vs_[i] - lm * (double)ms_[i];
        if (r0 > 0) {
            bp.push_back({r0 / (double)ls_[i], (double)qs[i] * (double)ls_[i]});
            S += (double)qs[i] * (double)ls_[i];
        }
    }
    if (S <= (double)Lcap) return 0;
    sort(bp.begin(), bp.end());
    for (auto& p : bp) {
        S -= p.second;
        if (S <= (double)Lcap) return p.first;
    }
    return bp.empty() ? 0 : bp.back().first;
}

static double evalL(double lm, double ll_) {
    double res = lm * (double)Mcap + ll_ * (double)Lcap;
    for (int i = 0; i < n; i++) {
        double r = (double)vs_[i] - lm * (double)ms_[i] - ll_ * (double)ls_[i];
        if (r > 0) res += (double)qs[i] * r;
    }
    return res;
}

static void findRootLambda() {
    double lm = 0, ll_ = 0;
    double prev = 1e30;
    for (int it = 0; it < 80; it++) {
        double newLm = optLambdaMfull(ll_);
        double newLl = optLambdaLfull(newLm);
        double cur = evalL(newLm, newLl);
        if (cur > prev - 1e-9) { lm = newLm; ll_ = newLl; break; }
        prev = cur;
        lm = newLm; ll_ = newLl;
    }
    lambda_m_root = lm;
    lambda_l_root = ll_;
}

static ll bestVal;
static vector<ll> best_x, cur_x;

// Per-node tight Lagrangian dual UB. Coord descent on (lm, ll_) for the
// residual subproblem (items idx..n-1, capacities mLeft/lLeft).
static double upperBound(int idx, ll mLeft, ll lLeft) {
    double lm = 0, ll_ = 0;
    pair<double,double> bp_buf[64];
    for (int it = 0; it < 6; it++) {
        // Optimize lm for fixed ll_ using piecewise-linear breakpoints.
        int nb = 0; double S = 0;
        for (int i = idx; i < n; i++) {
            double r0 = (double)vs_[i] - ll_ * (double)ls_[i];
            if (r0 > 0) {
                bp_buf[nb++] = {r0 / (double)ms_[i], (double)qs[i] * (double)ms_[i]};
                S += (double)qs[i] * (double)ms_[i];
            }
        }
        double newLm;
        if (S <= (double)mLeft) newLm = 0;
        else {
            sort(bp_buf, bp_buf + nb);
            newLm = bp_buf[nb - 1].first;
            for (int k = 0; k < nb; k++) {
                S -= bp_buf[k].second;
                if (S <= (double)mLeft) { newLm = bp_buf[k].first; break; }
            }
        }
        // Optimize ll_ for fixed newLm.
        nb = 0; double S2 = 0;
        for (int i = idx; i < n; i++) {
            double r0 = (double)vs_[i] - newLm * (double)ms_[i];
            if (r0 > 0) {
                bp_buf[nb++] = {r0 / (double)ls_[i], (double)qs[i] * (double)ls_[i]};
                S2 += (double)qs[i] * (double)ls_[i];
            }
        }
        double newLl;
        if (S2 <= (double)lLeft) newLl = 0;
        else {
            sort(bp_buf, bp_buf + nb);
            newLl = bp_buf[nb - 1].first;
            for (int k = 0; k < nb; k++) {
                S2 -= bp_buf[k].second;
                if (S2 <= (double)lLeft) { newLl = bp_buf[k].first; break; }
            }
        }
        if (abs(newLm - lm) < 1e-15 && abs(newLl - ll_) < 1e-15) {
            lm = newLm; ll_ = newLl;
            break;
        }
        lm = newLm; ll_ = newLl;
    }
    double ub = lm * (double)mLeft + ll_ * (double)lLeft;
    for (int i = idx; i < n; i++) {
        double r = (double)vs_[i] - lm * (double)ms_[i] - ll_ * (double)ls_[i];
        if (r > 0) ub += (double)qs[i] * r;
    }
    return ub;
}

static int timeCheckCounter = 0;
static bool timeUp = false;

static void bnb(int idx, ll mLeft, ll lLeft, ll val) {
    if (val > bestVal) { bestVal = val; best_x = cur_x; }
    if (idx == n) return;
    if ((++timeCheckCounter & 2047) == 0) {
        if (elapsed() > TIME_LIMIT) timeUp = true;
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

    // Reorder all arrays by v/(m+l) desc using normalized capacity weights.
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            long double da = (long double)ms_[a] / (long double)Mcap + (long double)ls_[a] / (long double)Lcap;
            long double db = (long double)ms_[b] / (long double)Mcap + (long double)ls_[b] / (long double)Lcap;
            return (long double)vs_[a] * db > (long double)vs_[b] * da;
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

    // Heuristic phase.
    bestVal = 0;
    best_x.assign(n, 0);
    auto tryX = [&](vector<ll> x) {
        localSearch(x);
        ll v = value(x);
        if (v > bestVal) { bestVal = v; best_x = x; }
    };

    vector<double> alphas;
    for (int k = 0; k <= 100; k++) alphas.push_back(k / 100.0);
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

    // Find root Lagrangian dual multipliers.
    findRootLambda();

    // Reorder by reduced cost (v - lm*m - ll*l) desc for tighter B&B pruning.
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            double ra = (double)vs_[a] - lambda_m_root * (double)ms_[a] - lambda_l_root * (double)ls_[a];
            double rb = (double)vs_[b] - lambda_m_root * (double)ms_[b] - lambda_l_root * (double)ls_[b];
            if (ra != rb) return ra > rb;
            return (long double)vs_[a] * ms_[b] > (long double)vs_[b] * ms_[a];
        });
        vector<string> nN(n); vector<ll> nQ(n), nV(n), nM(n), nL(n), nBx(n);
        for (int k = 0; k < n; k++) {
            int o = ord[k];
            nN[k] = names[o]; nQ[k] = qs[o]; nV[k] = vs_[o]; nM[k] = ms_[o]; nL[k] = ls_[o];
            nBx[k] = best_x[o];
        }
        names = nN; qs = nQ; vs_ = nV; ms_ = nM; ls_ = nL; best_x = nBx;
    }

    // Branch and bound.
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
