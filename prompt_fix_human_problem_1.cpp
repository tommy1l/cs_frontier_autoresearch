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

ll greedy_fill(const vector<Item>& items, const vector<int>& order, vector<ll>& cnt) {
    cnt.assign(items.size(), 0);
    ll usedM = 0, usedL = 0, val = 0;
    for (int idx : order) {
        const auto& it = items[idx];
        ll capM = (MAX_MASS - usedM) / it.m;
        ll capL = (MAX_VOL - usedL) / it.l;
        ll take = min({it.q, capM, capL});
        if (take > 0) {
            cnt[idx] = take;
            usedM += take * it.m;
            usedL += take * it.l;
            val += take * it.v;
        }
    }
    return val;
}

ll local_2opt(const vector<Item>& items, vector<ll>& cnt) {
    int n = items.size();
    ll usedM = 0, usedL = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        usedM += cnt[i] * items[i].m;
        usedL += cnt[i] * items[i].l;
        val += cnt[i] * items[i].v;
    }
    bool improved = true;
    int rounds = 0;
    while (improved && rounds < 200 && elapsed_ms() < TIME_MS) {
        improved = false;
        for (int i = 0; i < n && elapsed_ms() < TIME_MS; ++i) {
            for (int j = i + 1; j < n; ++j) {
                ll Mavail = MAX_MASS - usedM + cnt[i]*items[i].m + cnt[j]*items[j].m;
                ll Vavail = MAX_VOL - usedL + cnt[i]*items[i].l + cnt[j]*items[j].l;
                ll maxA = min({items[i].q, Mavail / items[i].m, Vavail / items[i].l});
                ll bestA = cnt[i], bestB = cnt[j];
                ll bestPair = cnt[i]*items[i].v + cnt[j]*items[j].v;
                for (ll a = 0; a <= maxA; ++a) {
                    ll Mr = Mavail - a*items[i].m, Vr = Vavail - a*items[i].l;
                    if (Mr < 0 || Vr < 0) break;
                    ll b = min({items[j].q, Mr/items[j].m, Vr/items[j].l});
                    ll v = a*items[i].v + b*items[j].v;
                    if (v > bestPair) { bestPair = v; bestA = a; bestB = b; }
                }
                if (bestA != cnt[i] || bestB != cnt[j]) {
                    usedM += (bestA - cnt[i])*items[i].m + (bestB - cnt[j])*items[j].m;
                    usedL += (bestA - cnt[i])*items[i].l + (bestB - cnt[j])*items[j].l;
                    val += (bestA - cnt[i])*items[i].v + (bestB - cnt[j])*items[j].v;
                    cnt[i] = bestA; cnt[j] = bestB;
                    improved = true;
                }
            }
        }
        ++rounds;
    }
    return val;
}

// Lagrangian relaxation sweep: for each lambda, sort by v / (m + lambda*l)
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    vector<ll> bestCnt(n, 0);
    ll bestVal = 0;

    vector<double> lambdas;
    for (double lam = 0.0; lam <= 5.0; lam += 0.02) lambdas.push_back(lam);

    for (double lam : lambdas) {
        if (elapsed_ms() > TIME_MS / 3) break;
        vector<int> order(n);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) {
            double ra = (double)items[a].v / (items[a].m + lam * items[a].l);
            double rb = (double)items[b].v / (items[b].m + lam * items[b].l);
            return ra > rb;
        });
        vector<ll> cnt;
        ll v = greedy_fill(items, order, cnt);
        if (v > bestVal) { bestVal = v; bestCnt = cnt; }
    }

    ll v = local_2opt(items, bestCnt);
    if (v > bestVal) bestVal = v;

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
