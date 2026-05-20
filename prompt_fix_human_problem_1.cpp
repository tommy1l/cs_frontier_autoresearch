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

// 3-opt local search: for each triple (i, j, k), sample c_i and c_j,
// compute best c_k. Try to improve.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(54321);

    // Greedy seed
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });
    vector<ll> cnt(n, 0);
    ll uM = 0, uL = 0, val = 0;
    for (int i : ord) {
        ll t = min({items[i].q, (MAX_MASS - uM)/items[i].m, (MAX_VOL - uL)/items[i].l});
        if (t > 0) { cnt[i] = t; uM += t*items[i].m; uL += t*items[i].l; val += t*items[i].v; }
    }

    auto two_opt = [&]() {
        bool imp = true;
        while (imp && elapsed_ms() < TIME_MS) {
            imp = false;
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    ll Mav = MAX_MASS - uM + cnt[i]*items[i].m + cnt[j]*items[j].m;
                    ll Vav = MAX_VOL - uL + cnt[i]*items[i].l + cnt[j]*items[j].l;
                    ll maxA = min({items[i].q, Mav/items[i].m, Vav/items[i].l});
                    ll bA = cnt[i], bB = cnt[j];
                    ll bP = cnt[i]*items[i].v + cnt[j]*items[j].v;
                    for (ll a = 0; a <= maxA; ++a) {
                        ll Mr = Mav - a*items[i].m, Vr = Vav - a*items[i].l;
                        if (Mr < 0 || Vr < 0) break;
                        ll b = min({items[j].q, Mr/items[j].m, Vr/items[j].l});
                        ll v = a*items[i].v + b*items[j].v;
                        if (v > bP) { bP = v; bA = a; bB = b; }
                    }
                    if (bA != cnt[i] || bB != cnt[j]) {
                        uM += (bA - cnt[i])*items[i].m + (bB - cnt[j])*items[j].m;
                        uL += (bA - cnt[i])*items[i].l + (bB - cnt[j])*items[j].l;
                        val += (bA - cnt[i])*items[i].v + (bB - cnt[j])*items[j].v;
                        cnt[i] = bA; cnt[j] = bB;
                        imp = true;
                    }
                }
            }
        }
    };
    two_opt();

    // 3-opt with sampling
    auto three_opt = [&]() {
        bool imp = true;
        int SAMP = 30;
        while (imp && elapsed_ms() < TIME_MS) {
            imp = false;
            for (int i = 0; i < n && elapsed_ms() < TIME_MS; ++i) {
                for (int j = i + 1; j < n && elapsed_ms() < TIME_MS; ++j) {
                    for (int k = j + 1; k < n; ++k) {
                        ll Mav = MAX_MASS - uM + cnt[i]*items[i].m + cnt[j]*items[j].m + cnt[k]*items[k].m;
                        ll Vav = MAX_VOL - uL + cnt[i]*items[i].l + cnt[j]*items[j].l + cnt[k]*items[k].l;
                        ll maxA = min({items[i].q, Mav/items[i].m, Vav/items[i].l});
                        ll bA = cnt[i], bB = cnt[j], bC = cnt[k];
                        ll bP = cnt[i]*items[i].v + cnt[j]*items[j].v + cnt[k]*items[k].v;
                        for (int s = 0; s <= SAMP; ++s) {
                            ll a = maxA * s / SAMP;
                            ll Mr1 = Mav - a*items[i].m, Vr1 = Vav - a*items[i].l;
                            if (Mr1 < 0 || Vr1 < 0) break;
                            ll maxB = min({items[j].q, Mr1/items[j].m, Vr1/items[j].l});
                            for (int t = 0; t <= SAMP; ++t) {
                                ll b = maxB * t / SAMP;
                                ll Mr2 = Mr1 - b*items[j].m, Vr2 = Vr1 - b*items[j].l;
                                if (Mr2 < 0 || Vr2 < 0) break;
                                ll c = min({items[k].q, Mr2/items[k].m, Vr2/items[k].l});
                                ll v = a*items[i].v + b*items[j].v + c*items[k].v;
                                if (v > bP) { bP = v; bA = a; bB = b; bC = c; }
                            }
                        }
                        if (bA != cnt[i] || bB != cnt[j] || bC != cnt[k]) {
                            uM += (bA-cnt[i])*items[i].m + (bB-cnt[j])*items[j].m + (bC-cnt[k])*items[k].m;
                            uL += (bA-cnt[i])*items[i].l + (bB-cnt[j])*items[j].l + (bC-cnt[k])*items[k].l;
                            val += (bA-cnt[i])*items[i].v + (bB-cnt[j])*items[j].v + (bC-cnt[k])*items[k].v;
                            cnt[i] = bA; cnt[j] = bB; cnt[k] = bC;
                            imp = true;
                        }
                    }
                }
            }
            if (imp) two_opt();
        }
    };
    three_opt();

    cout << "{";
    bool first = true;
    int i = 0;
    for (auto& kv : data) {
        if (!first) cout << ",";
        cout << "\n  \"" << kv.first << "\": " << cnt[i];
        first = false;
        ++i;
    }
    cout << "\n}\n";
    return 0;
}
