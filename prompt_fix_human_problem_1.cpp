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

// Branch and bound on item counts in density-sorted order.
// Upper bound via LP relaxation: greedily fill remaining with fractional taking.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    // Sort items by density v/(m+l)
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });

    // Initial lower bound: greedy
    vector<ll> bestCnt(n, 0);
    ll bestVal = 0;
    {
        ll uM = 0, uL = 0;
        for (int i : ord) {
            ll t = min({items[i].q, (MAX_MASS - uM)/items[i].m, (MAX_VOL - uL)/items[i].l});
            if (t > 0) {
                bestCnt[i] = t; uM += t*items[i].m; uL += t*items[i].l; bestVal += t*items[i].v;
            }
        }
    }

    // LP upper bound (fractional knapsack on remaining)
    function<double(int, ll, ll)> upperBound = [&](int idx, ll rM, ll rL) -> double {
        double ub = 0;
        for (int k = idx; k < n; ++k) {
            int i = ord[k];
            double tM = (double)rM / items[i].m;
            double tL = (double)rL / items[i].l;
            double t = min({(double)items[i].q, tM, tL});
            if (t <= 0) continue;
            ub += t * items[i].v;
            rM -= (ll)(t * items[i].m);
            rL -= (ll)(t * items[i].l);
            if (rM <= 0 || rL <= 0) break;
        }
        return ub;
    };

    // Recursive B&B
    vector<ll> cur(n, 0);
    function<void(int, ll, ll, ll)> dfs = [&](int idx, ll rM, ll rL, ll val) {
        if (elapsed_ms() > TIME_MS) return;
        if (idx == n) {
            if (val > bestVal) { bestVal = val; bestCnt = cur; }
            return;
        }
        if (val + upperBound(idx, rM, rL) <= bestVal) return;
        int i = ord[idx];
        ll maxT = min({items[i].q, rM/items[i].m, rL/items[i].l});
        // Branch from maxT down to 0 (try big first for better pruning)
        for (ll t = maxT; t >= 0; --t) {
            cur[i] = t;
            dfs(idx + 1, rM - t*items[i].m, rL - t*items[i].l, val + t*items[i].v);
            if (elapsed_ms() > TIME_MS) return;
        }
        cur[i] = 0;
    };

    dfs(0, MAX_MASS, MAX_VOL, 0);

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
