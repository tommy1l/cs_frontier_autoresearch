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

// Beam search: branch on item types in sorted order, each level pick count.
// Keep top-K beams by value.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    // Sort items by value density descending for branching order
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });

    struct State { ll m, l, val; vector<ll> cnt; };
    vector<State> beam;
    beam.push_back({0, 0, 0, vector<ll>(n, 0)});

    const int BEAM_WIDTH = 2000;
    const int BRANCH_FACTOR = 8;

    for (int step = 0; step < n; ++step) {
        if (elapsed_ms() > TIME_MS - 50) break;
        int idx = ord[step];
        vector<State> next;
        next.reserve(beam.size() * BRANCH_FACTOR);
        for (auto& s : beam) {
            ll maxT = min({items[idx].q, (MAX_MASS - s.m) / items[idx].m,
                           (MAX_VOL - s.l) / items[idx].l});
            if (maxT < 0) maxT = 0;
            // Try a few candidate counts: 0, maxT, and BRANCH_FACTOR-2 equispaced
            set<ll> candidates;
            candidates.insert(0);
            candidates.insert(maxT);
            for (int k = 1; k < BRANCH_FACTOR - 1; ++k) {
                candidates.insert(maxT * k / (BRANCH_FACTOR - 1));
            }
            for (ll t : candidates) {
                State ns = s;
                ns.cnt[idx] = t;
                ns.m += t * items[idx].m;
                ns.l += t * items[idx].l;
                ns.val += t * items[idx].v;
                next.push_back(move(ns));
            }
        }
        if ((int)next.size() > BEAM_WIDTH) {
            nth_element(next.begin(), next.begin() + BEAM_WIDTH, next.end(),
                [](const State& a, const State& b) { return a.val > b.val; });
            next.resize(BEAM_WIDTH);
        }
        beam = move(next);
    }

    State best = beam[0];
    for (auto& s : beam) if (s.val > best.val) best = s;

    // Final polish: 2-opt
    auto& cnt = best.cnt;
    ll usedM = best.m, usedL = best.l, val = best.val;
    bool improved = true;
    while (improved && elapsed_ms() < TIME_MS) {
        improved = false;
        for (int i = 0; i < n && elapsed_ms() < TIME_MS; ++i) {
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
