#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    int Kmin = 0; { long long t = L; while (t) { Kmin++; t >>= 1; } }
    int Kmax = 0; { long long t = R; while (t) { Kmax++; t >>= 1; } }

    int nodeCount = 0;
    vector<vector<pair<int,int>>> edges;
    map<tuple<int, long long, long long>, int> atomNode;

    function<int(int, long long, long long)> getAtom = [&](int r, long long lo, long long hi) -> int {
        if (lo > hi) return -1;
        auto key = make_tuple(r, lo, hi);
        auto it = atomNode.find(key);
        if (it != atomNode.end()) return it->second;
        int id = nodeCount++;
        edges.push_back({});
        atomNode[key] = id;
        if (r >= 1) {
            long long shift = 1LL << (r - 1);
            for (int b = 0; b < 2; b++) {
                long long b_shift = (long long)b * shift;
                long long nlo = max(0LL, lo - b_shift);
                long long nhi = min(shift - 1, hi - b_shift);
                int child = getAtom(r - 1, nlo, nhi);
                if (child != -1) {
                    edges[id].push_back({child, b});
                }
            }
        }
        return id;
    };

    getAtom(0, 0, 0);
    int START = nodeCount++;
    edges.push_back({});

    for (int k = Kmin; k <= Kmax; k++) {
        long long L_k = max(L, 1LL << (k - 1));
        long long R_k = min(R, (1LL << k) - 1);
        if (L_k > R_k) continue;
        long long nlo = L_k - (1LL << (k - 1));
        long long nhi = R_k - (1LL << (k - 1));
        int atom = getAtom(k - 1, nlo, nhi);
        if (atom != -1) {
            edges[START].push_back({atom, 1});
        }
    }

    int N = nodeCount;
    vector<int> rep(N);
    iota(rep.begin(), rep.end(), 0);
    function<int(int)> findRep = [&](int x) {
        while (rep[x] != x) { rep[x] = rep[rep[x]]; x = rep[x]; }
        return x;
    };

    bool changed = true;
    while (changed) {
        changed = false;
        map<vector<pair<int,int>>, int> sig;
        for (int i = 0; i < N; i++) {
            if (i == START) continue;
            if (findRep(i) != i) continue;
            vector<pair<int,int>> v;
            for (auto& [a, w] : edges[i]) {
                v.push_back({w, findRep(a)});
            }
            sort(v.begin(), v.end());
            auto it = sig.find(v);
            if (it == sig.end()) {
                sig[v] = i;
            } else {
                rep[i] = it->second;
                changed = true;
            }
        }
    }

    vector<int> compact(N, -1);
    int newCount = 0;
    for (int i = 0; i < N; i++) {
        if (findRep(i) == i) {
            compact[i] = newCount++;
        }
    }

    cout << newCount << "\n";
    for (int i = 0; i < N; i++) {
        if (findRep(i) != i) continue;
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) {
            int target = findRep(a);
            cout << " " << (compact[target] + 1) << " " << w;
        }
        cout << "\n";
    }

    return 0;
}
