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

// Variable Neighborhood Search:
//  N1: 1-opt (each coordinate maximized)
//  N2: 2-opt pair re-allocation
//  N3: 3-coord shake
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(99999);

    auto recompute = [&](const vector<ll>& cnt, ll& m, ll& l, ll& v) {
        m = l = v = 0;
        for (int i = 0; i < n; ++i) {
            m += cnt[i] * items[i].m;
            l += cnt[i] * items[i].l;
            v += cnt[i] * items[i].v;
        }
    };

    auto n1_opt = [&](vector<ll>& cnt, ll& uM, ll& uL, ll& val) {
        bool imp = true;
        while (imp) {
            imp = false;
            for (int i = 0; i < n; ++i) {
                ll mAvail = MAX_MASS - uM + cnt[i] * items[i].m;
                ll vAvail = MAX_VOL - uL + cnt[i] * items[i].l;
                ll best = min({items[i].q, mAvail / items[i].m, vAvail / items[i].l});
                if (best != cnt[i]) {
                    ll d = best - cnt[i];
                    uM += d * items[i].m;
                    uL += d * items[i].l;
                    val += d * items[i].v;
                    cnt[i] = best;
                    imp = true;
                }
            }
        }
    };

    auto n2_opt = [&](vector<ll>& cnt, ll& uM, ll& uL, ll& val) {
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

    // Seed with greedy
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)items[a].v / (items[a].m + items[a].l) >
               (double)items[b].v / (items[b].m + items[b].l);
    });
    vector<ll> bestCnt(n, 0);
    ll bm = 0, bl = 0;
    for (int i : ord) {
        ll t = min({items[i].q, (MAX_MASS - bm)/items[i].m, (MAX_VOL - bl)/items[i].l});
        if (t > 0) { bestCnt[i] = t; bm += t*items[i].m; bl += t*items[i].l; }
    }
    ll bestVal;
    recompute(bestCnt, bm, bl, bestVal);
    n1_opt(bestCnt, bm, bl, bestVal);
    n2_opt(bestCnt, bm, bl, bestVal);

    // VNS shake-and-search
    while (elapsed_ms() < TIME_MS) {
        for (int k = 2; k <= 5 && elapsed_ms() < TIME_MS; ++k) {
            vector<ll> cur = bestCnt;
            for (int s = 0; s < k; ++s) {
                int idx = rng() % n;
                cur[idx] = rng() % (items[idx].q + 1);
            }
            ll cm, cl, cv;
            recompute(cur, cm, cl, cv);
            if (cm > MAX_MASS || cl > MAX_VOL) continue;
            n1_opt(cur, cm, cl, cv);
            n2_opt(cur, cm, cl, cv);
            if (cv > bestVal) {
                bestVal = cv;
                bestCnt = cur;
                bm = cm; bl = cl;
                break;
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
