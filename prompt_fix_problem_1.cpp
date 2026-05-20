// Treasure Packing: 2D bounded knapsack
// Approach: Pareto-frontier dynamic programming maintaining non-dominated (mass, vol, value) triples.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<long long> q, v, mm, ll;
static const long long M_CAP = 20000000LL;
static const long long L_CAP = 25000000LL;

struct State { long long mass, vol, val; int parent_idx; int taken; };

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);

    string s((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    vector<string> names;
    int pos = 0;
    while ((int)names.size() < 12) {
        while (pos < (int)s.size() && s[pos] != '"') pos++;
        if (pos >= (int)s.size()) break;
        int start = ++pos;
        while (pos < (int)s.size() && s[pos] != '"') pos++;
        if (pos >= (int)s.size()) break;
        string name = s.substr(start, pos - start);
        pos++;
        while (pos < (int)s.size() && s[pos] != '[') pos++;
        if (pos >= (int)s.size()) break;
        pos++;
        long long nums[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; ++i) {
            while (pos < (int)s.size() && !isdigit((unsigned char)s[pos])) pos++;
            long long val = 0;
            while (pos < (int)s.size() && isdigit((unsigned char)s[pos])) {
                val = val * 10 + (s[pos] - '0');
                pos++;
            }
            nums[i] = val;
        }
        names.push_back(name);
        q.push_back(nums[0]);
        v.push_back(nums[1]);
        mm.push_back(nums[2]);
        ll.push_back(nums[3]);
        while (pos < (int)s.size() && s[pos] != ']') pos++;
        if (pos < (int)s.size()) pos++;
    }
    n = (int)names.size();

    auto start_time = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - start_time).count(); };

    // Order items: process most-constraining first to prune aggressively
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) {
        return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) >
               (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
    });

    const int MAX_STATES = 250000;

    // layers[i] = list of states after processing items ord[0..i)
    vector<vector<State>> layers(n + 1);
    layers[0].push_back({0, 0, 0, -1, 0});

    bool budget_hit = false;
    for (int t = 0; t < n; ++t) {
        if (elapsed() > 0.7) { budget_hit = true; break; }
        int idx = ord[t];
        // Expand each state with k = 0..q[idx]
        // To keep tractable, limit copies per branch by mass/vol cap
        vector<State> nxt;
        nxt.reserve(min((size_t)MAX_STATES, layers[t].size() * 8));
        for (int pi = 0; pi < (int)layers[t].size(); ++pi) {
            const State& st = layers[t][pi];
            long long maxm = mm[idx] > 0 ? (M_CAP - st.mass) / mm[idx] : q[idx];
            long long maxv = ll[idx] > 0 ? (L_CAP - st.vol)  / ll[idx] : q[idx];
            long long maxk = min({q[idx], maxm, maxv});
            // Subsample k values when too many
            int step = 1;
            if (maxk > 200) step = (int)(maxk / 200) + 1;
            for (long long k = 0; k <= maxk; k += step) {
                nxt.push_back({st.mass + k * mm[idx], st.vol + k * ll[idx], st.val + k * v[idx], pi, (int)k});
            }
            if (maxk % step != 0) {
                long long k = maxk;
                nxt.push_back({st.mass + k * mm[idx], st.vol + k * ll[idx], st.val + k * v[idx], pi, (int)k});
            }
        }
        // Prune to Pareto frontier
        sort(nxt.begin(), nxt.end(), [](const State& a, const State& b) {
            if (a.mass != b.mass) return a.mass < b.mass;
            if (a.vol  != b.vol)  return a.vol  < b.vol;
            return a.val > b.val;
        });
        vector<State> pruned;
        pruned.reserve(nxt.size());
        // For each (mass, vol), keep highest val; then dominance prune
        long long best_v_for_low_mass = -1;
        // Need 2D Pareto: state (m, l, v) dominated if exists (m', l', v') with m' <= m, l' <= l, v' >= v
        // Sweep: sort by mass asc, then sweep maintaining best val per vol-prefix
        // Simpler: O(N^2) check (limited N)
        int N = (int)nxt.size();
        if (N > MAX_STATES) {
            // Keep only highest-val states
            sort(nxt.begin(), nxt.end(), [](const State& a, const State& b) { return a.val > b.val; });
            nxt.resize(MAX_STATES);
            sort(nxt.begin(), nxt.end(), [](const State& a, const State& b) {
                if (a.mass != b.mass) return a.mass < b.mass;
                if (a.vol  != b.vol)  return a.vol  < b.vol;
                return a.val > b.val;
            });
            N = MAX_STATES;
        }
        for (int i = 0; i < N; ++i) {
            bool dominated = false;
            for (int j = 0; j < (int)pruned.size(); ++j) {
                if (pruned[j].mass <= nxt[i].mass && pruned[j].vol <= nxt[i].vol && pruned[j].val >= nxt[i].val) {
                    if (pruned[j].mass < nxt[i].mass || pruned[j].vol < nxt[i].vol || pruned[j].val > nxt[i].val) {
                        dominated = true; break;
                    }
                }
            }
            if (!dominated) pruned.push_back(nxt[i]);
            if ((int)pruned.size() > MAX_STATES) break;
        }
        layers[t + 1] = std::move(pruned);
    }

    // Find best state
    vector<int> take(n, 0);
    long long best_val = -1;
    int best_idx = -1, best_layer = (budget_hit ? -1 : n);
    if (best_layer < 0) {
        // Use last computed layer
        for (int l = n; l >= 0; --l) if (!layers[l].empty()) { best_layer = l; break; }
    }
    for (int i = 0; i < (int)layers[best_layer].size(); ++i) {
        if (layers[best_layer][i].val > best_val) { best_val = layers[best_layer][i].val; best_idx = i; }
    }
    // Reconstruct
    int cur_layer = best_layer, cur_idx = best_idx;
    while (cur_layer > 0 && cur_idx >= 0) {
        int idx_item = ord[cur_layer - 1];
        take[idx_item] = layers[cur_layer][cur_idx].taken;
        cur_idx = layers[cur_layer][cur_idx].parent_idx;
        cur_layer--;
    }

    // Fill any remaining capacity greedily (since we subsampled k)
    long long mass = 0, vol = 0;
    for (int i = 0; i < n; ++i) { mass += (long long)take[i] * mm[i]; vol += (long long)take[i] * ll[i]; }
    {
        vector<int> idx(n);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) {
            return (double)v[a] / (mm[a] / (double)M_CAP + ll[a] / (double)L_CAP) >
                   (double)v[b] / (mm[b] / (double)M_CAP + ll[b] / (double)L_CAP);
        });
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i : idx) {
                while (take[i] < q[i] && mass + mm[i] <= M_CAP && vol + ll[i] <= L_CAP) {
                    take[i]++; mass += mm[i]; vol += ll[i]; improved = true;
                }
            }
        }
    }

    cout << "{\n";
    for (int i = 0; i < n; ++i) {
        cout << " \"" << names[i] << "\": " << take[i];
        if (i + 1 < n) cout << ",";
        cout << "\n";
    }
    cout << "}\n";
    return 0;
}
