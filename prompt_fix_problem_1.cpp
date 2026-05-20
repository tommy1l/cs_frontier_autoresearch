// Treasure Packing: 2D bounded knapsack
// Approach: Beam search over items. For each beam state, expand by trying multiple quantities of the next item.
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

    // Process items by value-density descending
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) >
               (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
    });

    struct Beam { vector<int> take; long long mass, vol, val; };
    vector<Beam> beam;
    beam.push_back({vector<int>(n, 0), 0LL, 0LL, 0LL});

    const int BEAM_WIDTH = 200;
    const int CHOICES_PER_STATE = 12;

    for (int step = 0; step < n; ++step) {
        int idx = ord[step];
        vector<Beam> next_beam;
        next_beam.reserve(beam.size() * CHOICES_PER_STATE);
        for (auto& b : beam) {
            long long maxm = mm[idx] > 0 ? (M_CAP - b.mass) / mm[idx] : q[idx];
            long long maxv = ll[idx] > 0 ? (L_CAP - b.vol)  / ll[idx] : q[idx];
            long long maxk = min({q[idx], maxm, maxv});
            if (maxk < 0) maxk = 0;
            // Try CHOICES_PER_STATE different k values
            set<long long> ks;
            ks.insert(0); ks.insert(maxk);
            for (int c = 1; c < CHOICES_PER_STATE - 1; ++c) {
                long long k = (maxk * c) / (CHOICES_PER_STATE - 1);
                ks.insert(k);
            }
            for (long long k : ks) {
                Beam nb = b;
                nb.take[idx] = (int)k;
                nb.mass += k * mm[idx];
                nb.vol  += k * ll[idx];
                nb.val  += k * v[idx];
                next_beam.push_back(std::move(nb));
            }
        }
        // Keep top BEAM_WIDTH by val
        if ((int)next_beam.size() > BEAM_WIDTH) {
            nth_element(next_beam.begin(), next_beam.begin() + BEAM_WIDTH, next_beam.end(),
                        [](const Beam& a, const Beam& b) { return a.val > b.val; });
            next_beam.resize(BEAM_WIDTH);
        }
        beam = std::move(next_beam);
    }

    // Pick best from final beam
    long long best_val = -1;
    vector<int> best;
    for (auto& b : beam) {
        if (b.val > best_val) { best_val = b.val; best = b.take; }
    }

    // Polish: greedy fill + 1-for-k swap
    {
        long long mass = 0, vol = 0;
        for (int i = 0; i < n; ++i) { mass += (long long)best[i] * mm[i]; vol += (long long)best[i] * ll[i]; }
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 0; i < n; ++i) {
                while (best[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    best[i]++; mass += mm[i]; vol += ll[i]; improved = true;
                }
            }
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                if (best[i] >= q[i] || best[j] == 0) continue;
                long long need_m = mass + mm[i] - M_CAP;
                long long need_v = vol  + ll[i] - L_CAP;
                long long k = 0;
                if (need_m > 0) k = max(k, (need_m + mm[j] - 1) / mm[j]);
                if (need_v > 0) k = max(k, (need_v + ll[j] - 1) / ll[j]);
                if (k > best[j]) continue;
                long long dval = v[i] - k * v[j];
                if (dval > 0) {
                    best[i]++; best[j] -= (int)k;
                    mass += mm[i] - k * mm[j]; vol += ll[i] - k * ll[j];
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
