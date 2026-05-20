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

// Tabu search: at each step, try all single-coordinate moves to all possible
// counts, accepting the best non-tabu (or aspiration). Diversify with restarts.
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(31415);

    auto greedy_seed = [&](double lam) {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            double sa = (double)items[a].v / (items[a].m + lam * items[a].l);
            double sb = (double)items[b].v / (items[b].m + lam * items[b].l);
            return sa > sb;
        });
        vector<ll> cnt(n, 0);
        ll uM = 0, uL = 0;
        for (int i : ord) {
            ll t = min({items[i].q, (MAX_MASS - uM) / items[i].m,
                        (MAX_VOL - uL) / items[i].l});
            if (t > 0) { cnt[i] = t; uM += t*items[i].m; uL += t*items[i].l; }
        }
        return cnt;
    };

    vector<ll> bestCnt = greedy_seed(1.0);
    ll bestVal = 0;
    for (int i = 0; i < n; ++i) bestVal += bestCnt[i] * items[i].v;

    int restart_count = 0;
    while (elapsed_ms() < TIME_MS) {
        // Seed with varied lambda
        double lam = (rng() % 1000) / 200.0;
        vector<ll> cnt = greedy_seed(lam);
        ll uM = 0, uL = 0, val = 0;
        for (int i = 0; i < n; ++i) {
            uM += cnt[i] * items[i].m;
            uL += cnt[i] * items[i].l;
            val += cnt[i] * items[i].v;
        }

        // Tabu list: (item index, count) -> recently visited
        vector<int> tabu(n, 0);
        const int TENURE = 5;
        int iter = 0;
        ll localBest = val;
        vector<ll> localBestCnt = cnt;

        while (elapsed_ms() < TIME_MS && iter < 30) {
            // Find best single-coord move
            ll bestDelta = LLONG_MIN;
            int bestI = -1;
            ll bestNew = 0;
            for (int i = 0; i < n; ++i) {
                if (tabu[i] > iter) continue;
                ll mAvail = MAX_MASS - uM + cnt[i]*items[i].m;
                ll vAvail = MAX_VOL - uL + cnt[i]*items[i].l;
                ll maxNew = min({items[i].q, mAvail/items[i].m, vAvail/items[i].l});
                // Try maxNew and a few alternates
                vector<ll> candidates = {maxNew, 0, maxNew/2, cnt[i]+1, cnt[i]-1};
                for (ll nv : candidates) {
                    if (nv < 0 || nv > items[i].q) continue;
                    if (nv > maxNew) continue;
                    if (nv == cnt[i]) continue;
                    ll delta = (nv - cnt[i]) * items[i].v;
                    if (delta > bestDelta) {
                        bestDelta = delta;
                        bestI = i;
                        bestNew = nv;
                    }
                }
            }
            if (bestI < 0) break;
            uM += (bestNew - cnt[bestI]) * items[bestI].m;
            uL += (bestNew - cnt[bestI]) * items[bestI].l;
            val += bestDelta;
            cnt[bestI] = bestNew;
            tabu[bestI] = iter + TENURE;
            if (val > localBest) { localBest = val; localBestCnt = cnt; }
            ++iter;
        }
        if (localBest > bestVal) { bestVal = localBest; bestCnt = localBestCnt; }
        ++restart_count;
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
