// Treasure Packing: 2D bounded knapsack
// Approach: Genetic algorithm with uniform crossover, mutation, tournament selection, and repair operator
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

// Repair an infeasible chromosome by trimming lowest-density items, then fill remaining capacity
void repair_and_fill(vector<int>& t) {
    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) { mass += (long long)t[i] * mm[i]; vol += (long long)t[i] * ll[i]; }
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) <
               (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
    });
    int idx = 0;
    while (mass > M_CAP || vol > L_CAP) {
        if (idx >= n) break;
        int i = order[idx];
        if (t[i] > 0) {
            long long over_m = max(0LL, mass - M_CAP);
            long long over_v = max(0LL, vol - L_CAP);
            long long need = 0;
            if (over_m > 0 && mm[i] > 0) need = max(need, (over_m + mm[i] - 1) / mm[i]);
            if (over_v > 0 && ll[i] > 0) need = max(need, (over_v + ll[i] - 1) / ll[i]);
            long long can = min((long long)t[i], need);
            if (can <= 0) { idx++; continue; }
            t[i] -= (int)can;
            mass -= can * mm[i];
            vol  -= can * ll[i];
        } else {
            idx++;
        }
    }
    // Fill: pass through best->worst density and pack as much as possible
    reverse(order.begin(), order.end());
    for (int i : order) {
        long long maxm = mm[i] > 0 ? (M_CAP - mass) / mm[i] : q[i];
        long long maxv = ll[i] > 0 ? (L_CAP - vol)  / ll[i] : q[i];
        long long can = min({(long long)(q[i] - t[i]), maxm, maxv});
        if (can > 0) {
            t[i] += (int)can;
            mass += can * mm[i];
            vol  += can * ll[i];
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

    mt19937_64 rng(0xBEEF1234);

    auto random_chrom = [&]() {
        vector<int> t(n, 0);
        for (int i = 0; i < n; ++i) t[i] = (int)(rng() % (q[i] + 1));
        repair_and_fill(t);
        return t;
    };

    int POP = 60;
    vector<vector<int>> pop(POP);
    vector<long long> fit(POP);
    for (int p = 0; p < POP; ++p) {
        pop[p] = random_chrom();
        fit[p] = evaluate(pop[p]);
    }

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    long long best_val = -1;
    vector<int> best;
    for (int p = 0; p < POP; ++p) if (fit[p] > best_val) { best_val = fit[p]; best = pop[p]; }

    while (elapsed() < 0.7) {
        // Tournament select two parents
        auto tourney = [&]() {
            int a = (int)(rng() % POP), b = (int)(rng() % POP);
            return fit[a] > fit[b] ? a : b;
        };
        int pa = tourney(), pb = tourney();
        // Uniform crossover
        vector<int> child(n);
        for (int i = 0; i < n; ++i) child[i] = (rng() & 1) ? pop[pa][i] : pop[pb][i];
        // Mutation
        for (int i = 0; i < n; ++i) {
            if ((rng() % 100) < 10) {
                int range = 5 + (int)(rng() % 20);
                int delta = (int)(rng() % (2 * range + 1)) - range;
                int newv = child[i] + delta;
                if (newv < 0) newv = 0;
                if (newv > q[i]) newv = (int)q[i];
                child[i] = newv;
            }
        }
        repair_and_fill(child);
        long long cf = evaluate(child);
        if (cf > best_val) { best_val = cf; best = child; }
        // Replace worst in population
        int worst = 0;
        for (int p = 1; p < POP; ++p) if (fit[p] < fit[worst]) worst = p;
        if (cf > fit[worst]) { pop[worst] = child; fit[worst] = cf; }
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
