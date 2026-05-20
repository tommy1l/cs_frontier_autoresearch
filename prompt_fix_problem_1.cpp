// Treasure Packing: 2D bounded knapsack, 12 categories
// Approach: multi-greedy start, then simulated annealing exploring a wider neighborhood
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

long long evaluate(const vector<int>& take) {
    long long mass = 0, vol = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        mass += (long long)take[i] * mm[i];
        vol  += (long long)take[i] * ll[i];
        val  += (long long)take[i] * v[i];
    }
    if (mass > M_CAP || vol > L_CAP) return -1;
    return val;
}

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

    vector<vector<int>> cands;
    for (int wi = 0; wi <= 40; ++wi) {
        double w = wi / 40.0;
        cands.push_back(greedy_by([&, w](int i) {
            return (double)v[i] / (w * mm[i] / (double)M_CAP + (1.0 - w) * ll[i] / (double)L_CAP);
        }));
    }
    cands.push_back(greedy_by([&](int i) { return (double)v[i] / (double)mm[i]; }));
    cands.push_back(greedy_by([&](int i) { return (double)v[i] / (double)ll[i]; }));
    cands.push_back(greedy_by([&](int i) {
        return (double)v[i] / max(mm[i] / (double)M_CAP, ll[i] / (double)L_CAP);
    }));

    vector<int> best = cands[0];
    long long best_val = evaluate(best);
    for (auto& c : cands) {
        long long vv = evaluate(c);
        if (vv > best_val) { best_val = vv; best = c; }
    }

    auto compute_state = [&](const vector<int>& t) {
        long long mass = 0, vol = 0, val = 0;
        for (int i = 0; i < n; ++i) {
            mass += (long long)t[i] * mm[i];
            vol  += (long long)t[i] * ll[i];
            val  += (long long)t[i] * v[i];
        }
        return tuple<long long, long long, long long>(mass, vol, val);
    };

    // Simulated annealing starting from best greedy
    mt19937_64 rng(0xC0FFEE);
    vector<int> cur = best;
    auto [cur_mass, cur_vol, cur_val] = compute_state(cur);
    long long best_overall_val = best_val;
    vector<int> best_overall = best;

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() {
        return chrono::duration<double>(chrono::steady_clock::now() - start_time).count();
    };

    double TIME_LIMIT = 0.75;
    double T0 = 1e7;
    long long iter = 0;
    while (elapsed() < TIME_LIMIT) {
        iter++;
        double frac = elapsed() / TIME_LIMIT;
        double T = T0 * pow(1e-4, frac);

        int move_type = (int)(rng() % 100);
        int i = (int)(rng() % n);

        if (move_type < 40) {
            // Adjust single category by small delta
            int range = 3 + (int)(rng() % 5);
            int delta = (int)(rng() % (2 * range + 1)) - range;
            if (delta == 0) continue;
            long long new_take = (long long)cur[i] + delta;
            if (new_take < 0 || new_take > q[i]) continue;
            long long nm = cur_mass + (long long)delta * mm[i];
            long long nv = cur_vol  + (long long)delta * ll[i];
            if (nm > M_CAP || nv > L_CAP) continue;
            long long nval = cur_val + (long long)delta * v[i];
            long long dval = nval - cur_val;
            double accept = dval >= 0 ? 1.0 : exp((double)dval / T);
            if ((double)rng() / (double)rng.max() < accept) {
                cur[i] = (int)new_take;
                cur_mass = nm; cur_vol = nv; cur_val = nval;
                if (cur_val > best_overall_val) { best_overall_val = cur_val; best_overall = cur; }
            }
        } else if (move_type < 80) {
            // Swap: remove a few of i, add up to capacity of j
            int j = (int)(rng() % n);
            if (i == j) continue;
            if (cur[i] == 0) continue;
            int remove = 1 + (int)(rng() % min((long long)5, (long long)cur[i]));
            long long free_mass = M_CAP - cur_mass + (long long)remove * mm[i];
            long long free_vol  = L_CAP - cur_vol  + (long long)remove * ll[i];
            long long max_add_m = mm[j] > 0 ? free_mass / mm[j] : q[j];
            long long max_add_v = ll[j] > 0 ? free_vol  / ll[j] : q[j];
            long long max_add = min({q[j] - (long long)cur[j], max_add_m, max_add_v});
            if (max_add <= 0) continue;
            long long add = 1 + (long long)(rng() % (max_add));
            if (add > max_add) add = max_add;
            long long dval = add * v[j] - (long long)remove * v[i];
            double accept = dval >= 0 ? 1.0 : exp((double)dval / T);
            if ((double)rng() / (double)rng.max() < accept) {
                cur[i] -= remove;
                cur[j] += (int)add;
                cur_mass += add * mm[j] - (long long)remove * mm[i];
                cur_vol  += add * ll[j] - (long long)remove * ll[i];
                cur_val  += dval;
                if (cur_val > best_overall_val) { best_overall_val = cur_val; best_overall = cur; }
            }
        } else {
            // Kick: large random perturbation
            int delta = (int)(rng() % 21) - 10;
            if (delta == 0) continue;
            long long new_take = (long long)cur[i] + delta;
            if (new_take < 0) new_take = 0;
            if (new_take > q[i]) new_take = q[i];
            long long actual = new_take - cur[i];
            long long nm = cur_mass + actual * mm[i];
            long long nv = cur_vol  + actual * ll[i];
            if (nm > M_CAP || nv > L_CAP) continue;
            long long nval = cur_val + actual * v[i];
            long long dval = nval - cur_val;
            double accept = dval >= 0 ? 1.0 : exp((double)dval / T);
            if ((double)rng() / (double)rng.max() < accept) {
                cur[i] = (int)new_take;
                cur_mass = nm; cur_vol = nv; cur_val = nval;
                if (cur_val > best_overall_val) { best_overall_val = cur_val; best_overall = cur; }
            }
        }
    }

    best = best_overall;

    // Final deterministic polish: keep adding any items that still fit
    {
        long long mass = 0, vol = 0;
        for (int i = 0; i < n; ++i) { mass += (long long)best[i] * mm[i]; vol += (long long)best[i] * ll[i]; }
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n; ++i) {
                while (best[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    best[i]++;
                    mass += mm[i];
                    vol  += ll[i];
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
