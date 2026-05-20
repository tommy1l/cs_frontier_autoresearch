// Treasure Packing: 2D bounded knapsack
// Approach: quantized 2D DP with binary expansion, then refine exactness with local search
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

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

    const int MB = 200, VB = 200;
    const double mass_step = (double)M_CAP / MB;
    const double vol_step  = (double)L_CAP / VB;

    // Expand bounded items by powers-of-two grouping
    struct Group {
        int orig;
        long long g_v, g_m, g_l;
        int g_q;
    };
    vector<Group> groups;
    for (int i = 0; i < n; ++i) {
        long long rem = q[i];
        long long pwr = 1;
        while (rem > 0) {
            long long take = min(pwr, rem);
            groups.push_back({i, v[i] * take, mm[i] * take, ll[i] * take, (int)take});
            rem -= take;
            pwr *= 2;
        }
    }

    // DP: dp[mb][vb] = max value; parent[gi][mb][vb] = whether took
    vector<vector<long long>> dp(MB + 1, vector<long long>(VB + 1, 0));
    int G = (int)groups.size();
    vector<vector<vector<uint8_t>>> took(G, vector<vector<uint8_t>>(MB + 1, vector<uint8_t>(VB + 1, 0)));

    for (int gi = 0; gi < G; ++gi) {
        int dm = (int)ceil((double)groups[gi].g_m / mass_step);
        int dv = (int)ceil((double)groups[gi].g_l / vol_step);
        long long gv = groups[gi].g_v;
        for (int b = MB; b >= dm; --b) {
            for (int c = VB; c >= dv; --c) {
                long long alt = dp[b - dm][c - dv] + gv;
                if (alt > dp[b][c]) {
                    dp[b][c] = alt;
                    took[gi][b][c] = 1;
                }
            }
        }
    }

    // Trace back from dp[MB][VB]
    vector<int> take(n, 0);
    int cb = MB, cc = VB;
    for (int gi = G - 1; gi >= 0; --gi) {
        if (took[gi][cb][cc]) {
            take[groups[gi].orig] += groups[gi].g_q;
            int dm = (int)ceil((double)groups[gi].g_m / mass_step);
            int dv = (int)ceil((double)groups[gi].g_l / vol_step);
            cb -= dm; cc -= dv;
        }
    }

    // The quantization may make the result infeasible, so trim down where needed
    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) { mass += (long long)take[i] * mm[i]; vol += (long long)take[i] * ll[i]; }
    // If infeasible, greedily remove worst items
    while (mass > M_CAP || vol > L_CAP) {
        int worst = -1; double worst_score = 1e18;
        for (int i = 0; i < n; ++i) {
            if (take[i] == 0) continue;
            double sc = (double)v[i] / (mm[i] / (double)M_CAP + ll[i] / (double)L_CAP);
            if (sc < worst_score) { worst_score = sc; worst = i; }
        }
        if (worst < 0) break;
        take[worst]--;
        mass -= mm[worst]; vol -= ll[worst];
    }

    // Fill remaining capacity greedily
    {
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) {
            return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) >
                   (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
        });
        for (int i : idx) {
            long long maxm = (M_CAP - mass) / mm[i];
            long long maxv = (L_CAP - vol)  / ll[i];
            long long can = min({(long long)(q[i] - take[i]), maxm, maxv});
            if (can > 0) {
                take[i] += (int)can;
                mass += can * mm[i];
                vol  += can * ll[i];
            }
        }
    }

    // Local 1-for-k swap polish
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n; ++i) {
            if (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                take[i]++; mass += mm[i]; vol += ll[i]; improved = true;
            }
        }
        for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (take[i] >= q[i]) continue;
            if (take[j] == 0) continue;
            long long need_mass = mass + mm[i] - M_CAP;
            long long need_vol  = vol  + ll[i] - L_CAP;
            long long k = 0;
            if (need_mass > 0) k = max(k, (need_mass + mm[j] - 1) / mm[j]);
            if (need_vol > 0)  k = max(k, (need_vol  + ll[j] - 1) / ll[j]);
            if (k > take[j]) continue;
            long long dval = v[i] - k * v[j];
            if (dval > 0) {
                take[i]++; take[j] -= (int)k;
                mass += mm[i] - k * mm[j];
                vol  += ll[i] - k * ll[j];
                improved = true;
            }
        }
    }

    cout << "{\n";
    for (int i = 0; i < n; ++i) {
        cout << " \"" << names[i] << "\": " << take[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
