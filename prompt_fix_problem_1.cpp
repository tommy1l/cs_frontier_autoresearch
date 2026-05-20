// Treasure Packing: 2D bounded knapsack
// Approach: Lagrangian dual variable (lambda_m, lambda_l) grid search; for each pair, pick items
// whose v_i - lambda_m * m_i - lambda_l * l_i > 0, greedy-fill in that order, then polish.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

long long evaluate(const vector<int>& t) {
    long long mass = 0, vol = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        mass += (long long)t[i] * mm[i];
        vol  += (long long)t[i] * ll[i];
        val  += (long long)t[i] * v[i];
    }
    if (mass > M_CAP || vol > L_CAP) return -1;
    return val;
}

vector<int> lagrangian_greedy(double lam_m, double lam_l) {
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    // Score by reduced value per resource: prefer items with high (v - lam_m*m - lam_l*l)
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        double da = v[a] - lam_m * mm[a] - lam_l * ll[a];
        double db = v[b] - lam_m * mm[b] - lam_l * ll[b];
        // Tie-breaker: per-resource efficiency
        double ra = (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP);
        double rb = (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
        if (da != db) return da > db;
        return ra > rb;
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
}

void polish(vector<int>& take) {
    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) { mass += (long long)take[i] * mm[i]; vol += (long long)take[i] * ll[i]; }
    bool improved = true;
    int iters = 0;
    while (improved && iters < 200) {
        improved = false; iters++;
        for (int i = 0; i < n; ++i) {
            while (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                take[i]++; mass += mm[i]; vol += ll[i]; improved = true;
            }
        }
        for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (take[i] >= q[i] || take[j] == 0) continue;
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

    // Determine reasonable scale of dual variables from data
    double v_avg = 0, m_avg = 0, l_avg = 0;
    for (int i = 0; i < n; ++i) { v_avg += v[i]; m_avg += mm[i]; l_avg += ll[i]; }
    v_avg /= n; m_avg /= n; l_avg /= n;
    double lam_m_max = v_avg / max(1.0, m_avg) * 5.0;
    double lam_l_max = v_avg / max(1.0, l_avg) * 5.0;

    vector<int> best;
    long long best_val = -1;

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    int GRID = 30;
    for (int a = 0; a <= GRID; ++a) {
        for (int b = 0; b <= GRID; ++b) {
            if (elapsed() > 0.7) break;
            double lam_m = lam_m_max * a / GRID;
            double lam_l = lam_l_max * b / GRID;
            auto cand = lagrangian_greedy(lam_m, lam_l);
            polish(cand);
            long long vv = evaluate(cand);
            if (vv > best_val) { best_val = vv; best = cand; }
        }
        if (elapsed() > 0.7) break;
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
