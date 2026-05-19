#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int n;
vector<string> names;
vector<ll> Q, V, M, L;
const ll M_CAP = 20000000;
const ll L_CAP = 25000000;

ll eval_value(const vector<ll>& x, ll& mass, ll& vol) {
    ll val = 0; mass = 0; vol = 0;
    for (int i = 0; i < n; i++) {
        val += x[i] * V[i];
        mass += x[i] * M[i];
        vol += x[i] * L[i];
    }
    return val;
}

vector<ll> greedy_fill_order(const vector<int>& ord) {
    vector<ll> x(n, 0);
    ll mass = 0, vol = 0;
    for (int idx : ord) {
        ll max_m = (M_CAP - mass) / M[idx];
        ll max_l = (L_CAP - vol) / L[idx];
        ll take = min({Q[idx], max_m, max_l});
        x[idx] = take;
        mass += take * M[idx];
        vol += take * L[idx];
    }
    return x;
}

vector<ll> greedy_w(double w) {
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        double da = (double)V[a] / (w * (double)M[a] / M_CAP + (1 - w) * (double)L[a] / L_CAP);
        double db = (double)V[b] / (w * (double)M[b] / M_CAP + (1 - w) * (double)L[b] / L_CAP);
        return da > db;
    });
    return greedy_fill_order(ord);
}

bool swap_improve(vector<ll>& x, ll& val, ll& mass, ll& vol) {
    bool any = false;
    for (int j = 0; j < n; j++) {
        if (x[j] == 0) continue;
        for (int i = 0; i < n; i++) {
            if (i == j) continue;
            if (x[i] >= Q[i]) continue;

            ll best_delta = 0;
            ll best_rem = 0, best_add = 0;
            ll xj = x[j];
            for (ll rem = 1; rem <= xj; rem++) {
                ll new_mass = mass - rem * M[j];
                ll new_vol = vol - rem * L[j];
                ll max_m = (M_CAP - new_mass) / M[i];
                ll max_l = (L_CAP - new_vol) / L[i];
                ll can_add = min({Q[i] - x[i], max_m, max_l});
                if (can_add <= 0) continue;
                ll delta = can_add * V[i] - rem * V[j];
                if (delta > best_delta) {
                    best_delta = delta;
                    best_rem = rem;
                    best_add = can_add;
                }
            }
            if (best_delta > 0) {
                x[j] -= best_rem;
                x[i] += best_add;
                mass = mass - best_rem * M[j] + best_add * M[i];
                vol = vol - best_rem * L[j] + best_add * L[i];
                val += best_delta;
                any = true;
            }
        }
    }
    return any;
}

void local_search(vector<ll>& x, double time_limit_sec, chrono::steady_clock::time_point start) {
    ll mass, vol;
    ll val = eval_value(x, mass, vol);
    while (true) {
        if (chrono::duration<double>(chrono::steady_clock::now() - start).count() > time_limit_sec) break;
        if (!swap_improve(x, val, mass, vol)) break;
    }
}

int main() {
    auto start = chrono::steady_clock::now();
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    string input((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());

    size_t pos = 0;
    while (pos < input.size()) {
        size_t qs = input.find('"', pos);
        if (qs == string::npos) break;
        size_t qe = input.find('"', qs + 1);
        if (qe == string::npos) break;
        string key = input.substr(qs + 1, qe - qs - 1);
        pos = qe + 1;

        size_t lb = input.find('[', pos);
        if (lb == string::npos) break;
        size_t rb = input.find(']', lb);
        if (rb == string::npos) break;

        vector<ll> nums;
        size_t p = lb + 1;
        while (p < rb) {
            while (p < rb && !isdigit((unsigned char)input[p])) p++;
            if (p >= rb) break;
            size_t s = p;
            while (p < rb && isdigit((unsigned char)input[p])) p++;
            nums.push_back(stoll(input.substr(s, p - s)));
        }

        if (nums.size() == 4) {
            names.push_back(key);
            Q.push_back(nums[0]);
            V.push_back(nums[1]);
            M.push_back(nums[2]);
            L.push_back(nums[3]);
        }
        pos = rb + 1;
    }

    n = (int)names.size();

    vector<vector<ll>> starts;
    for (int wi = 0; wi <= 20; wi++) {
        double w = wi / 20.0;
        starts.push_back(greedy_w(w));
    }

    sort(starts.begin(), starts.end(), [&](const vector<ll>& a, const vector<ll>& b) {
        ll ma, va, mb, vb;
        return eval_value(a, ma, va) > eval_value(b, mb, vb);
    });

    vector<ll> best_sol = starts[0];
    ll mass, vol;
    ll best_val = eval_value(best_sol, mass, vol);

    const double TOTAL_BUDGET = 0.85;
    for (auto& s : starts) {
        double elapsed = chrono::duration<double>(chrono::steady_clock::now() - start).count();
        if (elapsed >= TOTAL_BUDGET) break;
        vector<ll> x = s;
        local_search(x, TOTAL_BUDGET, start);
        ll v = eval_value(x, mass, vol);
        if (v > best_val) {
            best_val = v;
            best_sol = x;
        }
    }
    best_val = eval_value(best_sol, mass, vol);

    cout << "{\n";
    for (int i = 0; i < n; i++) {
        cout << " \"" << names[i] << "\": " << best_sol[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";

    return 0;
}
