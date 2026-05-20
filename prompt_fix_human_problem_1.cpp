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

// GRASP: at each construction step, pick from restricted candidate list (RCL)
// of items with density within alpha of the max. Repeat many times, 2-opt each.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(1234567);

    auto construct = [&](double alpha, vector<ll>& cnt) {
        cnt.assign(n, 0);
        ll uM = 0, uL = 0, val = 0;
        vector<bool> used(n, false);
        while (true) {
            // Compute density for each unused item with available capacity
            vector<pair<double, int>> ranked;
            for (int i = 0; i < n; ++i) {
                if (used[i]) continue;
                ll mAvail = (MAX_MASS - uM) / items[i].m;
                ll lAvail = (MAX_VOL - uL) / items[i].l;
                ll maxT = min({items[i].q, mAvail, lAvail});
                if (maxT <= 0) { used[i] = true; continue; }
                double d = (double)items[i].v / (items[i].m + items[i].l);
                ranked.push_back({d, i});
            }
            if (ranked.empty()) break;
            sort(ranked.begin(), ranked.end(), greater<>());
            double maxD = ranked[0].first, minD = ranked.back().first;
            double thresh = maxD - alpha * (maxD - minD);
            int rcl_end = 0;
            while (rcl_end < (int)ranked.size() && ranked[rcl_end].first >= thresh) rcl_end++;
            int pick = ranked[rng() % rcl_end].second;
            ll mAvail = (MAX_MASS - uM) / items[pick].m;
            ll lAvail = (MAX_VOL - uL) / items[pick].l;
            ll t = min({items[pick].q, mAvail, lAvail});
            cnt[pick] = t;
            uM += t * items[pick].m;
            uL += t * items[pick].l;
            val += t * items[pick].v;
            used[pick] = true;
        }
        return val;
    };

    auto two_opt = [&](vector<ll>& cnt) {
        ll uM = 0, uL = 0, val = 0;
        for (int i = 0; i < n; ++i) {
            uM += cnt[i]*items[i].m; uL += cnt[i]*items[i].l; val += cnt[i]*items[i].v;
        }
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
        return val;
    };

    vector<ll> bestCnt(n, 0);
    ll bestVal = 0;
    while (elapsed_ms() < TIME_MS) {
        double alpha = 0.05 + 0.3 * ((double)rng() / rng.max());
        vector<ll> cnt;
        construct(alpha, cnt);
        ll v = two_opt(cnt);
        if (v > bestVal) { bestVal = v; bestCnt = cnt; }
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
