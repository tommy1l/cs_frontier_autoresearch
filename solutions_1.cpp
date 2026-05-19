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

pair<ll, ll> best_ab(int i, int k, ll Sm, ll Sl, ll Amax, ll Bmax) {
    ll best_val = 0;
    ll best_a = 0, best_b = 0;
    auto try_ab = [&](ll a, ll b) {
        if (a < 0 || b < 0 || a > Amax || b > Bmax) return;
        if (a*M[i] + b*M[k] > Sm) return;
        if (a*L[i] + b*L[k] > Sl) return;
        ll v = a*V[i] + b*V[k];
        if (v > best_val) { best_val = v; best_a = a; best_b = b; }
    };
    try_ab(0, 0);
    try_ab(Amax, 0);
    try_ab(0, Bmax);
    try_ab(Amax, Bmax);
    {
        ll rest_m = Sm - Amax*M[i];
        ll rest_l = Sl - Amax*L[i];
        if (rest_m >= 0 && rest_l >= 0) {
            ll b = min({Bmax, rest_m/M[k], rest_l/L[k]});
            try_ab(Amax, b);
        }
    }
    {
        ll rest_m = Sm - Bmax*M[k];
        ll rest_l = Sl - Bmax*L[k];
        if (rest_m >= 0 && rest_l >= 0) {
            ll a = min({Amax, rest_m/M[i], rest_l/L[i]});
            try_ab(a, Bmax);
        }
    }
    {
        ll b = min({Bmax, Sm/M[k], Sl/L[k]});
        try_ab(0, b);
    }
    {
        ll a = min({Amax, Sm/M[i], Sl/L[i]});
        try_ab(a, 0);
    }
    long long det = (long long)M[i]*L[k] - (long long)M[k]*L[i];
    if (det != 0) {
        double a_d = (double)(Sm * L[k] - Sl * M[k]) / det;
        double b_d = (double)(Sl * M[i] - Sm * L[i]) / det;
        if (a_d >= -1 && b_d >= -1) {
            for (ll da = -1; da <= 1; da++) {
                for (ll db = -1; db <= 1; db++) {
                    ll a = (ll)a_d + da;
                    ll b = (ll)b_d + db;
                    try_ab(a, b);
                }
            }
        }
    }
    return {best_a, best_b};
}

bool triple_improve(vector<ll>& x, ll& val, ll& mass, ll& vol) {
    bool any = false;
    for (int j = 0; j < n; j++) {
        if (x[j] == 0) continue;
        ll xj = x[j];
        for (int i = 0; i < n; i++) {
            if (i == j) continue;
            for (int k = i + 1; k < n; k++) {
                if (k == j) continue;
                ll Amax = Q[i] - x[i];
                ll Bmax = Q[k] - x[k];
                if (Amax < 0) Amax = 0;
                if (Bmax < 0) Bmax = 0;
                if (Amax == 0 && Bmax == 0) continue;

                ll best_delta = 0;
                ll best_rem = 0, best_a = 0, best_b = 0;
                ll step = max((ll)1, xj / 200);
                for (ll rem = 1; rem <= xj; rem += step) {
                    ll Sm = M_CAP - mass + rem * M[j];
                    ll Sl = L_CAP - vol + rem * L[j];
                    auto [a, b] = best_ab(i, k, Sm, Sl, Amax, Bmax);
                    ll delta = a*V[i] + b*V[k] - rem*V[j];
                    if (delta > best_delta) {
                        best_delta = delta;
                        best_rem = rem;
                        best_a = a;
                        best_b = b;
                    }
                }
                if (xj > 1) {
                    ll Sm = M_CAP - mass + xj * M[j];
                    ll Sl = L_CAP - vol + xj * L[j];
                    auto [a, b] = best_ab(i, k, Sm, Sl, Amax, Bmax);
                    ll delta = a*V[i] + b*V[k] - xj*V[j];
                    if (delta > best_delta) {
                        best_delta = delta;
                        best_rem = xj;
                        best_a = a;
                        best_b = b;
                    }
                }

                if (best_delta > 0) {
                    x[j] -= best_rem;
                    x[i] += best_a;
                    x[k] += best_b;
                    mass += -best_rem*M[j] + best_a*M[i] + best_b*M[k];
                    vol += -best_rem*L[j] + best_a*L[i] + best_b*L[k];
                    val += best_delta;
                    any = true;
                }
            }
        }
    }
    return any;
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
        bool a = swap_improve(x, val, mass, vol);
        if (chrono::duration<double>(chrono::steady_clock::now() - start).count() > time_limit_sec) break;
        bool b = triple_improve(x, val, mass, vol);
        if (!a && !b) break;
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

    vector<ll> best_sol(n, 0);
    ll best_val = 0;

    for (int wi = 0; wi <= 20; wi++) {
        double w = wi / 20.0;
        auto x = greedy_w(w);
        ll mass, vol;
        ll v = eval_value(x, mass, vol);
        if (v > best_val) {
            best_val = v;
            best_sol = x;
        }
    }

    local_search(best_sol, 0.85, start);
    ll mass, vol;
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
