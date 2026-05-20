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

// Pure simulated annealing on count vector
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(42);

    // Initial: greedy by v/(m+l)
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });
    vector<ll> cnt(n, 0);
    ll usedM = 0, usedL = 0, curVal = 0;
    for (int idx : order) {
        ll t = min({items[idx].q, (MAX_MASS - usedM) / items[idx].m,
                    (MAX_VOL - usedL) / items[idx].l});
        if (t > 0) {
            cnt[idx] = t;
            usedM += t * items[idx].m;
            usedL += t * items[idx].l;
            curVal += t * items[idx].v;
        }
    }

    vector<ll> bestCnt = cnt;
    ll bestVal = curVal;

    // SA loop
    double T0 = 1e7, T1 = 1.0;
    long long iters = 0;
    while (elapsed_ms() < TIME_MS) {
        double frac = (double)elapsed_ms() / TIME_MS;
        double T = T0 * pow(T1 / T0, frac);
        // Pick an item, change its count
        int i = rng() % n;
        ll old = cnt[i];
        // Available capacity excluding item i
        ll mAvail = MAX_MASS - usedM + old * items[i].m;
        ll vAvail = MAX_VOL - usedL + old * items[i].l;
        ll maxNew = min({items[i].q, mAvail / items[i].m, vAvail / items[i].l});
        if (maxNew <= 0) continue;
        ll nv = rng() % (maxNew + 1);
        ll delta = (nv - old) * items[i].v;
        if (delta >= 0 || (double)rng() / rng.max() < exp(delta / T)) {
            usedM += (nv - old) * items[i].m;
            usedL += (nv - old) * items[i].l;
            curVal += delta;
            cnt[i] = nv;
            if (curVal > bestVal) { bestVal = curVal; bestCnt = cnt; }
        }
        ++iters;
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
