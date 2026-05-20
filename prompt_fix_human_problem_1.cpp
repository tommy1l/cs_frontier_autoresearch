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

// Enumerate inclusion mask: which items are allowed (cnt>0). For each mask,
// try greedy fill (with several densities), keep best. Final 2-opt polish.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    vector<double> lams = {0.0, 0.5, 1.0, 1.5, 2.0, 3.0};
    vector<ll> bestCnt(n, 0);
    ll bestVal = 0;

    for (int mask = 1; mask < (1 << n); ++mask) {
        if (elapsed_ms() > TIME_MS - 50) break;
        for (double lam : lams) {
            vector<int> ord;
            for (int i = 0; i < n; ++i) if (mask & (1 << i)) ord.push_back(i);
            sort(ord.begin(), ord.end(), [&](int a, int b) {
                double sa = (double)items[a].v / (items[a].m + lam*items[a].l);
                double sb = (double)items[b].v / (items[b].m + lam*items[b].l);
                return sa > sb;
            });
            vector<ll> cnt(n, 0);
            ll uM = 0, uL = 0, val = 0;
            for (int i : ord) {
                ll t = min({items[i].q, (MAX_MASS - uM)/items[i].m, (MAX_VOL - uL)/items[i].l});
                if (t > 0) { cnt[i] = t; uM += t*items[i].m; uL += t*items[i].l; val += t*items[i].v; }
            }
            if (val > bestVal) { bestVal = val; bestCnt = cnt; }
        }
    }

    // 2-opt polish
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
