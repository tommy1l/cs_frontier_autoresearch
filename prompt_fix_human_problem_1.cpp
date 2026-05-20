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

// Repair an infeasible count vector by reducing items by density.
ll evaluate_repair(const vector<Item>& items, vector<ll>& cnt) {
    int n = items.size();
    ll usedM = 0, usedL = 0, val = 0;
    for (int i = 0; i < n; ++i) {
        cnt[i] = max(0LL, min(cnt[i], items[i].q));
        usedM += cnt[i] * items[i].m;
        usedL += cnt[i] * items[i].l;
        val += cnt[i] * items[i].v;
    }
    while (usedM > MAX_MASS || usedL > MAX_VOL) {
        // Drop one of the worst item (smallest v / (m+l)) that has positive count
        int worst = -1;
        double worstScore = 1e30;
        for (int i = 0; i < n; ++i) {
            if (cnt[i] > 0) {
                double sc = (double)items[i].v / (items[i].m + items[i].l);
                if (sc < worstScore) { worstScore = sc; worst = i; }
            }
        }
        if (worst < 0) break;
        --cnt[worst];
        usedM -= items[worst].m;
        usedL -= items[worst].l;
        val -= items[worst].v;
    }
    // Try to add more
    for (int i = 0; i < n; ++i) {
        ll add = min({items[i].q - cnt[i], (MAX_MASS - usedM) / items[i].m,
                      (MAX_VOL - usedL) / items[i].l});
        if (add > 0) {
            cnt[i] += add;
            usedM += add * items[i].m;
            usedL += add * items[i].l;
            val += add * items[i].v;
        }
    }
    return val;
}

// Genetic algorithm
int main() {
    auto data = parseInput();
    vector<Item> items;
    for (auto& kv : data)
        items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = items.size();

    mt19937_64 rng(12345);

    const int POP = 50;
    vector<pair<ll, vector<ll>>> population;

    // Seed: random greedy with shuffled order
    for (int p = 0; p < POP; ++p) {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        shuffle(ord.begin(), ord.end(), rng);
        vector<ll> cnt(n, 0);
        ll usedM = 0, usedL = 0, val = 0;
        for (int i : ord) {
            ll t = min({items[i].q, (MAX_MASS - usedM) / items[i].m,
                        (MAX_VOL - usedL) / items[i].l});
            if (t > 0) {
                cnt[i] = t;
                usedM += t * items[i].m;
                usedL += t * items[i].l;
                val += t * items[i].v;
            }
        }
        population.push_back({val, cnt});
    }

    // Evolve
    while (elapsed_ms() < TIME_MS) {
        // Tournament selection
        int a = rng() % POP, b = rng() % POP, c = rng() % POP, d = rng() % POP;
        int p1 = population[a].first >= population[b].first ? a : b;
        int p2 = population[c].first >= population[d].first ? c : d;
        // Uniform crossover
        vector<ll> child(n);
        for (int i = 0; i < n; ++i) {
            child[i] = (rng() & 1) ? population[p1].second[i] : population[p2].second[i];
        }
        // Mutation: pick one gene, perturb
        if (rng() % 3 == 0) {
            int g = rng() % n;
            ll q = items[g].q;
            child[g] = rng() % (q + 1);
        }
        ll val = evaluate_repair(items, child);
        // Replace worst
        int worst = 0;
        for (int i = 1; i < POP; ++i) {
            if (population[i].first < population[worst].first) worst = i;
        }
        if (val > population[worst].first) {
            population[worst] = {val, child};
        }
    }

    ll bestVal = 0;
    vector<ll> bestCnt;
    for (auto& p : population) {
        if (p.first > bestVal) { bestVal = p.first; bestCnt = p.second; }
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
