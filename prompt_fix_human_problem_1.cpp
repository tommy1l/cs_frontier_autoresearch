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

// Coordinate descent: for each item, find optimal count given others fixed.
ll coord_descent(const vector<Item>& items, vector<ll>& cnt, ll deadline_ms) {
    int n = items.size();
    ll usedM = 0, usedL = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        usedM += cnt[i] * items[i].m;
        usedL += cnt[i] * items[i].l;
        val += cnt[i] * items[i].v;
    }
    bool improved = true;
    while (improved && elapsed_ms() < deadline_ms) {
        improved = false;
        for (int i = 0; i < n && elapsed_ms() < deadline_ms; ++i) {
            // Free up item i
            ll mAvail = MAX_MASS - usedM + cnt[i] * items[i].m;
            ll vAvail = MAX_VOL - usedL + cnt[i] * items[i].l;
            ll best = min({items[i].q, mAvail / items[i].m, vAvail / items[i].l});
            if (best != cnt[i]) {
                ll delta = (best - cnt[i]);
                usedM += delta * items[i].m;
                usedL += delta * items[i].l;
                val += delta * items[i].v;
                cnt[i] = best;
                improved = true;
            }
        }
    }
    return val;
}

// 2-opt swap optimization
ll two_opt(const vector<Item>& items, vector<ll>& cnt, ll deadline_ms) {
    int n = items.size();
    ll usedM = 0, usedL = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        usedM += cnt[i] * items[i].m;
        usedL += cnt[i] * items[i].l;
        val += cnt[i] * items[i].v;
    }
    bool improved = true;
    while (improved && elapsed_ms() < deadline_ms) {
        improved = false;
        for (int i = 0; i < n && elapsed_ms() < deadline_ms; ++i) {
            for (int j = i + 1; j < n; ++j) {
                ll Mav = MAX_MASS - usedM + cnt[i]*items[i].m + cnt[j]*items[j].m;
                ll Vav = MAX_VOL - usedL + cnt[i]*items[i].l + cnt[j]*items[j].l;
                ll maxA = min({items[i].q, Mav / items[i].m, Vav / items[i].l});
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
                    usedM += (bA - cnt[i])*items[i].m + (bB - cnt[j])*items[j].m;
                    usedL += (bA - cnt[i])*items[i].l + (bB - cnt[j])*items[j].l;
                    val += (bA - cnt[i])*items[i].v + (bB - cnt[j])*items[j].v;
                    cnt[i] = bA; cnt[j] = bB;
                    improved = true;
                }
            }
        }
    }
    return val;
}

// Iterated local search: start from greedy, then loop: kick + local search.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(7777);

    // Initial: greedy by v/(m+l)
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });
    vector<ll> cnt(n, 0);
    ll usedM = 0, usedL = 0;
    for (int i : ord) {
        ll t = min({items[i].q, (MAX_MASS - usedM) / items[i].m,
                    (MAX_VOL - usedL) / items[i].l});
        if (t > 0) { cnt[i] = t; usedM += t * items[i].m; usedL += t * items[i].l; }
    }

    coord_descent(items, cnt, 100);
    ll bestVal = two_opt(items, cnt, 200);
    vector<ll> bestCnt = cnt;

    // ILS loop
    while (elapsed_ms() < TIME_MS) {
        // Kick: zero out 3 random items
        cnt = bestCnt;
        for (int k = 0; k < 3; ++k) {
            int i = rng() % n;
            cnt[i] = 0;
        }
        // Then random fill / coord descent
        ll dl1 = elapsed_ms() + 30;
        coord_descent(items, cnt, min((ll)TIME_MS, dl1));
        ll dl2 = elapsed_ms() + 80;
        ll v = two_opt(items, cnt, min((ll)TIME_MS, dl2));
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
