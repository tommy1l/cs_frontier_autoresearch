// Treasure Packing: 2D bounded knapsack
// Approach: Differential Evolution. Population of candidate quantity vectors; trial = a + F*(b-c),
// crossover with current, repair to feasibility, accept if better.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

long long compute_val(const vector<int>& t) {
    long long val = 0;
    for (int i = 0; i < n; ++i) val += (long long)t[i] * v[i];
    return val;
}

void repair_and_fill(vector<int>& t) {
    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) {
        if (t[i] < 0) t[i] = 0;
        if (t[i] > q[i]) t[i] = (int)q[i];
        mass += (long long)t[i] * mm[i]; vol += (long long)t[i] * ll[i];
    }
    auto density = [&](int i) {
        return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
    };
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return density(a) < density(b); });
    int idx = 0;
    while (mass > M_CAP || vol > L_CAP) {
        if (idx >= n) break;
        int i = ord[idx];
        if (t[i] > 0) { t[i]--; mass -= mm[i]; vol -= ll[i]; }
        else idx++;
    }
    reverse(ord.begin(), ord.end());
    for (int i : ord) {
        long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
        long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
        long long can = min({(long long)(q[i] - t[i]), maxm, maxv});
        if (can > 0) {
            t[i] += (int)can;
            mass += can * mm[i]; vol += can * ll[i];
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

    mt19937_64 rng(0xCAFED00D);
    int POP = 40;
    vector<vector<int>> pop(POP, vector<int>(n, 0));
    vector<long long> fit(POP);
    for (int p = 0; p < POP; ++p) {
        for (int i = 0; i < n; ++i) pop[p][i] = (int)(rng() % (q[i] + 1));
        repair_and_fill(pop[p]);
        fit[p] = compute_val(pop[p]);
    }

    vector<int> best = pop[0];
    long long best_val = fit[0];
    for (int p = 1; p < POP; ++p) if (fit[p] > best_val) { best_val = fit[p]; best = pop[p]; }

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    double F = 0.7, CR = 0.9;
    while (elapsed() < 0.7) {
        for (int i = 0; i < POP; ++i) {
            int a, b, c;
            do { a = (int)(rng() % POP); } while (a == i);
            do { b = (int)(rng() % POP); } while (b == i || b == a);
            do { c = (int)(rng() % POP); } while (c == i || c == a || c == b);
            vector<int> trial(n);
            int forced = (int)(rng() % n);
            for (int j = 0; j < n; ++j) {
                double r = (double)rng() / (double)rng.max();
                if (r < CR || j == forced) {
                    double val = pop[a][j] + F * ((double)pop[b][j] - (double)pop[c][j]);
                    trial[j] = (int)round(val);
                } else {
                    trial[j] = pop[i][j];
                }
            }
            repair_and_fill(trial);
            long long tf = compute_val(trial);
            if (tf > fit[i]) {
                pop[i] = trial;
                fit[i] = tf;
                if (tf > best_val) { best_val = tf; best = trial; }
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
