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

// Portfolio approach: run 4 diverse heuristics, each in a slice of time budget,
// take the best result. Heuristics: greedy by v/m, by v/l, by v/(m+l), and
// randomized greedy with multi-restart and 2-opt polish.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(98765);

    auto greedy_order = [&](vector<int> ord, vector<ll>& cnt) {
        cnt.assign(n, 0);
        ll uM = 0, uL = 0, val = 0;
        for (int i : ord) {
            ll t = min({items[i].q, (MAX_MASS - uM)/items[i].m, (MAX_VOL - uL)/items[i].l});
            if (t > 0) { cnt[i] = t; uM += t*items[i].m; uL += t*items[i].l; val += t*items[i].v; }
        }
        return val;
    };

    auto two_opt = [&](vector<ll>& cnt, ll deadline) {
        ll uM = 0, uL = 0, val = 0;
        for (int i = 0; i < n; ++i) {
            uM += cnt[i]*items[i].m; uL += cnt[i]*items[i].l; val += cnt[i]*items[i].v;
        }
        bool imp = true;
        while (imp && elapsed_ms() < deadline) {
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

    // Heuristic 1: greedy by v/m
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return (double)items[a].v/items[a].m > (double)items[b].v/items[b].m;
        });
        vector<ll> c;
        greedy_order(ord, c);
        ll v = two_opt(c, TIME_MS / 4);
        if (v > bestVal) { bestVal = v; bestCnt = c; }
    }
    // Heuristic 2: greedy by v/l
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return (double)items[a].v/items[a].l > (double)items[b].v/items[b].l;
        });
        vector<ll> c;
        greedy_order(ord, c);
        ll v = two_opt(c, TIME_MS / 2);
        if (v > bestVal) { bestVal = v; bestCnt = c; }
    }
    // Heuristic 3: greedy by v/(m+l)
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return (double)items[a].v/(items[a].m+items[a].l) >
                   (double)items[b].v/(items[b].m+items[b].l);
        });
        vector<ll> c;
        greedy_order(ord, c);
        ll v = two_opt(c, 3 * TIME_MS / 4);
        if (v > bestVal) { bestVal = v; bestCnt = c; }
    }
    // Heuristic 4: randomized restarts of shuffled greedy + 2-opt
    while (elapsed_ms() < TIME_MS) {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        shuffle(ord.begin(), ord.end(), rng);
        vector<ll> c;
        greedy_order(ord, c);
        ll v = two_opt(c, TIME_MS);
        if (v > bestVal) { bestVal = v; bestCnt = c; }
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
