#include <bits/stdc++.h>
using namespace std;

// Iter 9: signature-based bottom-up minimization (Hopcroft-style).
// Build all (B,Lo,Hi) states bottom-up, then compute equivalence classes via hashing.

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    int L, R;
    cin >> L >> R;

    int bL = 0, bR = 0;
    for (int x = L; x; x >>= 1) bL++;
    for (int x = R; x; x >>= 1) bR++;

    // Collect required states top-down per width.
    vector<set<pair<int,int>>> intervals(bR + 1);

    auto addInitial = [&]() {
        if (bL == bR) {
            int B = bL;
            int mid = 1 << (B - 1);
            intervals[B - 1].insert({L - mid, R - mid});
        } else {
            int midL = 1 << (bL - 1);
            intervals[bL - 1].insert({L - midL, midL - 1});
            for (int B = bL + 1; B < bR; B++) {
                intervals[B - 1].insert({0, (1 << (B - 1)) - 1});
            }
            int midR = 1 << (bR - 1);
            intervals[bR - 1].insert({0, R - midR});
        }
    };
    addInitial();
    for (int B = bR - 1; B >= 1; B--) {
        for (auto& [Lo, Hi] : intervals[B]) {
            int mid = 1 << (B - 1);
            if (Lo <= min(Hi, mid - 1))
                intervals[B - 1].insert({Lo, min(Hi, mid - 1)});
            if (Hi >= mid)
                intervals[B - 1].insert({max(Lo, mid) - mid, Hi - mid});
        }
    }

    // Assign canonical IDs via signature.
    // signature for (B, Lo, Hi) = (B, child0_canonId or -1, child1_canonId or -1)
    map<tuple<int,int,int>, int> canon; // sig -> id
    map<tuple<int,int,int>, int> stateId; // (B,Lo,Hi) -> id
    int nextId = 0;

    // Bottom-up: width 0 first.
    // (0, 0, 0) is the only state at width 0.
    {
        auto sig = make_tuple(0, -1, -1);
        canon[sig] = nextId++;
        stateId[{0, 0, 0}] = canon[sig];
    }

    for (int B = 1; B < bR; B++) {
        for (auto& [Lo, Hi] : intervals[B]) {
            int mid = 1 << (B - 1);
            int c0 = -1, c1 = -1;
            if (Lo <= min(Hi, mid - 1)) c0 = stateId[{B - 1, Lo, min(Hi, mid - 1)}];
            if (Hi >= mid) c1 = stateId[{B - 1, max(Lo, mid) - mid, Hi - mid}];
            auto sig = make_tuple(B, c0, c1);
            auto it = canon.find(sig);
            int id;
            if (it == canon.end()) {
                id = nextId++;
                canon[sig] = id;
            } else id = it->second;
            stateId[{B, Lo, Hi}] = id;
        }
    }

    // Start node.
    int startId = nextId++;
    vector<vector<pair<int,int>>> edges(nextId);

    if (bL == bR) {
        int mid = 1 << (bL - 1);
        edges[startId].push_back({stateId[{bL - 1, L - mid, R - mid}], 1});
    } else {
        int midL = 1 << (bL - 1);
        edges[startId].push_back({stateId[{bL - 1, L - midL, midL - 1}], 1});
        for (int B = bL + 1; B < bR; B++) {
            edges[startId].push_back({stateId[{B - 1, 0, (1 << (B - 1)) - 1}], 1});
        }
        int midR = 1 << (bR - 1);
        edges[startId].push_back({stateId[{bR - 1, 0, R - midR}], 1});
    }

    // Internal edges (per canonical state).
    map<int, pair<int,int>> nodeChildren; // canon id -> (c0, c1)
    for (auto& [k, id] : canon) {
        auto [B, c0, c1] = k;
        nodeChildren[id] = {c0, c1};
    }
    for (auto& [id, ch] : nodeChildren) {
        if (id == 0) continue; // end state
        if (ch.first >= 0) edges[id].push_back({ch.first, 0});
        if (ch.second >= 0) edges[id].push_back({ch.second, 1});
    }

    // Reorder: put start first.
    int n = nextId;
    vector<int> perm(n);
    perm[startId] = 0;
    int idx = 1;
    for (int i = 0; i < n; i++) if (i != startId) perm[i] = idx++;
    vector<vector<pair<int,int>>> out(n);
    for (int i = 0; i < n; i++)
        for (auto& e : edges[i])
            out[perm[i]].push_back({perm[e.first], e.second});

    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) cout << " " << (e.first + 1) << " " << e.second;
        cout << "\n";
    }
    return 0;
}
