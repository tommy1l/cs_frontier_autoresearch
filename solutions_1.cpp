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
    if (Amax < 0) Amax = 0;
    if (Bmax < 0) Bmax = 0;
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
    if (Sm >= 0 && Sl >= 0) {
        ll b = min({Bmax, Sm/M[k], Sl/L[k]});
        try_ab(0, b);
        ll a = min({Amax, Sm/M[i], Sl/L[i]});
        try_ab(a, 0);
    }
    long long det = (long long)M[i]*L[k] - (long long)M[k]*L[i];
    if (det != 0 && Sm >= 0 && Sl >= 0) {
        double a_d = (double)((long long)Sm * L[k] - (long long)Sl * M[k]) / det;
        double b_d = (double)((long long)Sl * M[i] - (long long)Sm * L[i]) / det;
        if (a_d >= -1 && b_d >= -1) {
            for (ll da = -1; da <= 1; da++) {
                for (ll db = -1; db <= 1; db++) {
                    ll a = (ll)floor(a_d) + da;
                    ll b = (ll)floor(b_d) + db;
                    try_ab(a, b);
                }
            }
        }
    }
    return {best_a, best_b};
}

void lp_enumerate(vector<ll>& best_sol, ll& best_val) {
    for (int i = 0; i < n; i++) {
        for (int k = i + 1; k < n; k++) {
            vector<int> others;
            for (int j = 0; j < n; j++) if (j != i && j != k) others.push_back(j);
            int mo = (int)others.size();
            for (int mask = 0; mask < (1 << mo); mask++) {
                ll fixed_val = 0, fixed_m = 0, fixed_l = 0;
                bool ok = true;
                for (int b = 0; b < mo; b++) {
                    int j = others[b];
                    if (mask & (1 << b)) {
                        fixed_val += Q[j] * V[j];
                        fixed_m += Q[j] * M[j];
                        fixed_l += Q[j] * L[j];
                        if (fixed_m > M_CAP || fixed_l > L_CAP) { ok = false; break; }
                    }
                }
                if (!ok) continue;
                ll rem_m = M_CAP - fixed_m;
                ll rem_l = L_CAP - fixed_l;
                auto [a, b] = best_ab(i, k, rem_m, rem_l, Q[i], Q[k]);
                ll total = fixed_val + a * V[i] + b * V[k];
                if (total > best_val) {
                    best_val = total;
                    vector<ll> cur(n, 0);
                    for (int b2 = 0; b2 < mo; b2++) {
                        int j = others[b2];
                        if (mask & (1 << b2)) cur[j] = Q[j];
                    }
                    cur[i] = a;
                    cur[k] = b;
                    best_sol = cur;
                }
            }
        }
    }
}

bool swap_improve(vector<ll>& x, ll& val, ll& mass, ll& vol) {
    bool any = false;
    for (int i = 0; i < n; i++) {
        for (int k = i + 1; k < n; k++) {
            ll fixed_m = mass - x[i]*M[i] - x[k]*M[k];
            ll fixed_l = vol - x[i]*L[i] - x[k]*L[k];
            ll fixed_v = val - x[i]*V[i] - x[k]*V[k];
            ll rem_m = M_CAP - fixed_m;
            ll rem_l = L_CAP - fixed_l;
            if (rem_m < 0 || rem_l < 0) continue;
            auto [a, b] = best_ab(i, k, rem_m, rem_l, Q[i], Q[k]);
            ll new_val = fixed_v + a*V[i] + b*V[k];
            if (new_val > val) {
                ll dm = (a - x[i])*M[i] + (b - x[k])*M[k];
                ll dl = (a - x[i])*L[i] + (b - x[k])*L[k];
                val = new_val;
                mass += dm;
                vol += dl;
                x[i] = a;
                x[k] = b;
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

    vector<ll> best_sol(n, 0);
    ll best_val = 0;

    lp_enumerate(best_sol, best_val);

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
