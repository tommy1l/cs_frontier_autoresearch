#include <bits/stdc++.h>
using namespace std;
using ll = long long;

const ll MAX_MASS = 20LL * 1000 * 1000;
const ll MAX_VOL = 25LL * 1000 * 1000;
const int TIME_MS = 950;

struct Item { string name; ll q, v, m, l; };

map<string, vector<ll>> parseInput() {
    map<string, vector<ll>> data;
    char ch;
    cin >> ch;
    for (int i = 0; i < 12; ++i) {
        cin >> ch;
        string key;
        getline(cin, key, '"');
        cin >> ch;
        cin >> ch;
        ll q, v, m, l;
        cin >> q >> ch >> v >> ch >> m >> ch >> l;
        cin >> ch;
        data[key] = {q, v, m, l};
        if (i < 11) cin >> ch;
    }
    cin >> ch;
    return data;
}

static auto t_start = chrono::steady_clock::now();
ll elapsed_ms() {
    return chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now() - t_start).count();
}

// Cross-entropy method: maintain per-item count distribution (Gaussian-like
// using a "fraction" of max), sample candidates, update toward elite.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(2024);

    // Max feasible count per item
    vector<ll> maxc(n);
    for (int i = 0; i < n; ++i) {
        maxc[i] = min({items[i].q, MAX_MASS / items[i].m, MAX_VOL / items[i].l});
    }

    // Distribution: mean[i] in [0,1] fraction, std[i] in [0,1]
    vector<double> mean(n, 0.5), sigma(n, 0.3);

    const int SAMPLES = 200;
    const int ELITE = 30;

    vector<ll> bestCnt(n, 0);
    ll bestVal = 0;

    while (elapsed_ms() < TIME_MS - 100) {
        vector<pair<ll, vector<ll>>> samples;
        for (int s = 0; s < SAMPLES && elapsed_ms() < TIME_MS - 100; ++s) {
            vector<ll> cnt(n, 0);
            for (int i = 0; i < n; ++i) {
                double f = mean[i] + sigma[i] * (((double)rng() / rng.max()) * 2 - 1);
                f = clamp(f, 0.0, 1.0);
                cnt[i] = (ll)round(f * maxc[i]);
            }
            // Repair
            ll uM = 0, uL = 0;
            for (int i = 0; i < n; ++i) { uM += cnt[i]*items[i].m; uL += cnt[i]*items[i].l; }
            // If infeasible, reduce by density-low items
            while (uM > MAX_MASS || uL > MAX_VOL) {
                int worst = -1;
                double ws = 1e30;
                for (int i = 0; i < n; ++i) {
                    if (cnt[i] > 0) {
                        double sc = (double)items[i].v / (items[i].m + items[i].l);
                        if (sc < ws) { ws = sc; worst = i; }
                    }
                }
                if (worst < 0) break;
                --cnt[worst];
                uM -= items[worst].m;
                uL -= items[worst].l;
            }
            // Top-up greedy
            for (int i = 0; i < n; ++i) {
                ll add = min({items[i].q - cnt[i], (MAX_MASS - uM)/items[i].m, (MAX_VOL - uL)/items[i].l});
                if (add > 0) { cnt[i] += add; uM += add*items[i].m; uL += add*items[i].l; }
            }
            ll val = 0;
            for (int i = 0; i < n; ++i) val += cnt[i]*items[i].v;
            samples.push_back({val, cnt});
            if (val > bestVal) { bestVal = val; bestCnt = cnt; }
        }
        // Sort and take elite
        sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });
        if ((int)samples.size() < ELITE) break;
        // Update distribution
        for (int i = 0; i < n; ++i) {
            double sum = 0, sumSq = 0;
            for (int k = 0; k < ELITE; ++k) {
                double f = maxc[i] > 0 ? (double)samples[k].second[i] / maxc[i] : 0;
                sum += f; sumSq += f*f;
            }
            double newMean = sum / ELITE;
            double newVar = max(0.0, sumSq / ELITE - newMean*newMean);
            mean[i] = 0.7 * newMean + 0.3 * mean[i];
            sigma[i] = max(0.05, 0.7 * sqrt(newVar) + 0.3 * sigma[i]);
        }
    }

    // Final 2-opt polish
    ll uM = 0, uL = 0;
    for (int i = 0; i < n; ++i) { uM += bestCnt[i]*items[i].m; uL += bestCnt[i]*items[i].l; }
    bool imp = true;
    while (imp && elapsed_ms() < TIME_MS) {
        imp = false;
        for (int i = 0; i < n && elapsed_ms() < TIME_MS; ++i) {
            for (int j = i + 1; j < n; ++j) {
                ll Mav = MAX_MASS - uM + bestCnt[i]*items[i].m + bestCnt[j]*items[j].m;
                ll Vav = MAX_VOL - uL + bestCnt[i]*items[i].l + bestCnt[j]*items[j].l;
                ll maxA = min({items[i].q, Mav/items[i].m, Vav/items[i].l});
                ll bA = bestCnt[i], bB = bestCnt[j];
                ll bP = bestCnt[i]*items[i].v + bestCnt[j]*items[j].v;
                for (ll a = 0; a <= maxA; ++a) {
                    ll Mr = Mav - a*items[i].m, Vr = Vav - a*items[i].l;
                    if (Mr < 0 || Vr < 0) break;
                    ll b = min({items[j].q, Mr/items[j].m, Vr/items[j].l});
                    ll v = a*items[i].v + b*items[j].v;
                    if (v > bP) { bP = v; bA = a; bB = b; }
                }
                if (bA != bestCnt[i] || bB != bestCnt[j]) {
                    uM += (bA - bestCnt[i])*items[i].m + (bB - bestCnt[j])*items[j].m;
                    uL += (bA - bestCnt[i])*items[i].l + (bB - bestCnt[j])*items[j].l;
                    bestVal += (bA - bestCnt[i])*items[i].v + (bB - bestCnt[j])*items[j].v;
                    bestCnt[i] = bA; bestCnt[j] = bB;
                    imp = true;
                }
            }
        }
    }

    cout << "{";
    bool first = true;
    int i = 0;
    for (auto& kv : data) {
        if (!first) cout << ",";
        cout << "\n  \"" << kv.first << "\": " << bestCnt[i];
        first = false;
        ++i;
    }
    cout << "\n}\n";
    return 0;
}
