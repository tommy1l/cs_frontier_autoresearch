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

// For each pair (i, j) of items, enumerate counts of i and j (sampled),
// and greedy-fill the rest. Take best.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    auto greedy_rest = [&](vector<ll>& cnt, ll uM, ll uL) {
        // Sort remaining items by density
        vector<int> ord;
        for (int i = 0; i < n; ++i) if (cnt[i] == -1) ord.push_back(i);
        // Try a few densities, take best
        ll bestV = LLONG_MIN;
        vector<ll> bestC = cnt;
        vector<double> lams = {0.5, 1.0, 1.5, 2.0};
        for (double lam : lams) {
            vector<int> o = ord;
            sort(o.begin(), o.end(), [&](int a, int b) {
                return (double)items[a].v / (items[a].m + lam * items[a].l) >
                       (double)items[b].v / (items[b].m + lam * items[b].l);
            });
            vector<ll> tc = cnt;
            ll m2 = uM, l2 = uL, v2 = 0;
            for (int i = 0; i < n; ++i) if (tc[i] > 0) v2 += tc[i] * items[i].v;
            for (int i : o) {
                ll t = min({items[i].q, (MAX_MASS - m2) / items[i].m,
                            (MAX_VOL - l2) / items[i].l});
                tc[i] = (t > 0) ? t : 0;
                if (t > 0) { m2 += t*items[i].m; l2 += t*items[i].l; v2 += t*items[i].v; }
            }
            if (v2 > bestV) { bestV = v2; bestC = tc; }
        }
        cnt = bestC;
        return bestV;
    };

    ll bestVal = 0;
    vector<ll> bestCnt(n, 0);

    const int SAMPLES = 30;
    for (int i = 0; i < n; ++i) {
        if (elapsed_ms() > TIME_MS) break;
        for (int j = i + 1; j < n; ++j) {
            if (elapsed_ms() > TIME_MS) break;
            ll maxI = min({items[i].q, MAX_MASS / items[i].m, MAX_VOL / items[i].l});
            ll maxJ = min({items[j].q, MAX_MASS / items[j].m, MAX_VOL / items[j].l});
            for (int s = 0; s <= SAMPLES; ++s) {
                ll ci = maxI * s / SAMPLES;
                ll mI = ci * items[i].m, lI = ci * items[i].l;
                if (mI > MAX_MASS || lI > MAX_VOL) continue;
                for (int t = 0; t <= SAMPLES; ++t) {
                    ll cj = maxJ * t / SAMPLES;
                    ll mTot = mI + cj * items[j].m;
                    ll lTot = lI + cj * items[j].l;
                    if (mTot > MAX_MASS || lTot > MAX_VOL) continue;
                    vector<ll> cnt(n, -1);
                    cnt[i] = ci;
                    cnt[j] = cj;
                    ll v = greedy_rest(cnt, mTot, lTot);
                    if (v > bestVal) { bestVal = v; bestCnt = cnt; }
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
