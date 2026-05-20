// Treasure Packing: 2D bounded knapsack
// Approach: Tabu search. At each step, examine all 1-for-k swap moves, pick the best
// non-tabu move (or aspiration if it beats best so far). Maintain short tabu list of recent (i,j) pairs.
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

long long compute_val(const vector<int>& t, long long& mass_out, long long& vol_out) {
    long long mass = 0, vol = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        mass += (long long)t[i] * mm[i];
        vol  += (long long)t[i] * ll[i];
        val  += (long long)t[i] * v[i];
    }
    mass_out = mass; vol_out = vol;
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

    // Initial: best greedy across w-sweep
    vector<vector<int>> cands;
    for (int wi = 0; wi <= 40; ++wi) {
        double w = wi / 40.0;
        cands.push_back(greedy_by([&, w](int i) {
            return (double)v[i] / (w * mm[i] / (double)M_CAP + (1.0 - w) * ll[i] / (double)L_CAP);
        }));
    }
    vector<int> best = cands[0]; long long m_unused, v_unused;
    long long best_val = compute_val(best, m_unused, v_unused);
    for (auto& c : cands) {
        long long vv = compute_val(c, m_unused, v_unused);
        if (vv > best_val) { best_val = vv; best = c; }
    }

    vector<int> cur = best;
    long long cur_mass, cur_vol;
    long long cur_val = compute_val(cur, cur_mass, cur_vol);

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    // Tabu list: (i, j) pairs, with iteration when tabu expires
    vector<vector<int>> tabu_until(n, vector<int>(n, -1));
    int tabu_tenure = 7;
    int it = 0;

    while (elapsed() < 0.7) {
        it++;
        // Aspiration baseline
        long long best_move_dval = LLONG_MIN;
        int best_i = -1, best_j = -1; long long best_k = 0; int best_add = 0;
        bool aspiration = false;

        // Try add-one moves (no tabu)
        for (int i = 0; i < n; ++i) {
            if (cur[i] >= q[i]) continue;
            if (cur_mass + mm[i] > M_CAP || cur_vol + ll[i] > L_CAP) continue;
            long long dval = v[i];
            if (dval > best_move_dval) {
                best_move_dval = dval; best_i = i; best_j = -1; best_k = 0; best_add = 1;
            }
        }
        // Try 1-for-k swap (add 1 of i, remove k of j)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (cur[i] >= q[i] || cur[j] == 0) continue;
                long long need_m = cur_mass + mm[i] - M_CAP;
                long long need_v = cur_vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_m > 0 && mm[j] > 0) k = max(k, (need_m + mm[j] - 1) / mm[j]);
                if (need_v > 0 && ll[j] > 0) k = max(k, (need_v + ll[j] - 1) / ll[j]);
                if (k > cur[j]) continue;
                long long dval = v[i] - k * v[j];
                bool is_tabu = tabu_until[i][j] > it;
                long long new_total = cur_val + dval;
                bool aspirate = new_total > best_val;
                if (is_tabu && !aspirate) continue;
                if (dval > best_move_dval) {
                    best_move_dval = dval;
                    best_i = i; best_j = j; best_k = k; best_add = 1;
                    aspiration = aspirate;
                }
            }
        }
        // Try k-for-1 swap (add k of i, remove 1 of j)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (cur[j] == 0) continue;
                long long free_m = M_CAP - cur_mass + mm[j];
                long long free_v = L_CAP - cur_vol + ll[j];
                long long max_k_m = mm[i] > 0 ? free_m / mm[i] : q[i];
                long long max_k_v = ll[i] > 0 ? free_v / ll[i] : q[i];
                long long max_k = min({(long long)(q[i] - cur[i]), max_k_m, max_k_v});
                if (max_k < 1) continue;
                long long k = max_k;
                long long dval = k * v[i] - v[j];
                bool is_tabu = tabu_until[j][i] > it;
                long long new_total = cur_val + dval;
                bool aspirate = new_total > best_val;
                if (is_tabu && !aspirate) continue;
                if (dval > best_move_dval) {
                    best_move_dval = dval;
                    best_i = j; best_j = i; best_k = 1; best_add = (int)k;
                    aspiration = aspirate;
                }
            }
        }

        if (best_i < 0) break;
        if (best_move_dval == LLONG_MIN) break;

        // Apply move (interpretation): remove best_k of best_i, add best_add of best_j (if j>=0)
        // OR if best_j == -1, just add best_add of best_i
        if (best_j < 0) {
            cur[best_i] += best_add;
            cur_mass += best_add * mm[best_i];
            cur_vol  += best_add * ll[best_i];
            cur_val  += best_add * v[best_i];
        } else {
            cur[best_i] -= best_k;
            cur[best_j] += best_add;
            cur_mass += best_add * mm[best_j] - best_k * mm[best_i];
            cur_vol  += best_add * ll[best_j] - best_k * ll[best_i];
            cur_val  += best_add * v[best_j] - best_k * v[best_i];
            tabu_until[best_i][best_j] = it + tabu_tenure;
            tabu_until[best_j][best_i] = it + tabu_tenure;
        }
        if (cur_val > best_val) {
            best_val = cur_val;
            best = cur;
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
