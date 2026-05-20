// Treasure Packing: 2D bounded knapsack, 12 categories
// Constraints: total mass <= 20*10^6 mg, total volume <= 25*10^6 µL
// Approach: multi-greedy with various scoring functions + local search (1-for-k swap)
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
    cands.push_back(greedy_by([&](int i) {
        return (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
    }));
    cands.push_back(greedy_by([&](int i) { return (double)v[i] / (double)mm[i]; }));
    cands.push_back(greedy_by([&](int i) { return (double)v[i] / (double)ll[i]; }));
    cands.push_back(greedy_by([&](int i) {
        return (double)v[i] / max(mm[i] / (double)M_CAP, ll[i] / (double)L_CAP);
    }));
    for (int wi = 0; wi <= 20; ++wi) {
        double w = wi / 20.0;
        cands.push_back(greedy_by([&, w](int i) {
            return (double)v[i] / (w * mm[i] / (double)M_CAP + (1.0 - w) * ll[i] / (double)L_CAP);
        }));
    }

    vector<int> best = cands[0];
    long long best_val = evaluate(best);
    for (auto& c : cands) {
        long long vv = evaluate(c);
        if (vv > best_val) { best_val = vv; best = c; }
    }

    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) { mass += (long long)best[i] * mm[i]; vol += (long long)best[i] * ll[i]; }

    bool improved = true;
    int safety = 0;
    while (improved && safety < 10000) {
        improved = false;
        safety++;
        for (int i = 0; i < n; ++i) {
            if (best[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                best[i]++;
                mass += mm[i];
                vol  += ll[i];
                best_val += v[i];
                improved = true;
            }
        }
        if (improved) continue;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (best[i] >= q[i]) continue;
                if (best[j] == 0) continue;
                long long need_mass = mass + mm[i] - M_CAP;
                long long need_vol  = vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_mass > 0) {
                    if (mm[j] == 0) { k = LLONG_MAX; }
                    else k = max(k, (need_mass + mm[j] - 1) / mm[j]);
                }
                if (need_vol > 0) {
                    if (ll[j] == 0) { k = LLONG_MAX; }
                    else k = max(k, (need_vol + ll[j] - 1) / ll[j]);
                }
                if (k > best[j]) continue;
                long long delta = v[i] - k * v[j];
                if (delta > 0) {
                    best[i]++;
                    best[j] -= (int)k;
                    mass += mm[i] - k * mm[j];
                    vol  += ll[i] - k * ll[j];
                    best_val += delta;
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
