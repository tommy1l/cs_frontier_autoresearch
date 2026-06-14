#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R; cin >> L >> R;
    int Kmin = 0, Kmax = 0;
    { long long t = L; while (t) { Kmin++; t >>= 1; } }
    { long long t = R; while (t) { Kmax++; t >>= 1; } }

    vector<vector<pair<int,int>>> edges;
    auto newNode = [&]() { int id = (int)edges.size(); edges.push_back({}); return id; };

    int END = newNode();

    vector<int> W(Kmax + 2, -1);
    W[0] = END;
    function<int(int)> getW = [&](int r) -> int {
        if (W[r] != -1) return W[r];
        int c = getW(r - 1);
        int id = newNode();
        W[r] = id;
        edges[id].push_back({c, 0});
        edges[id].push_back({c, 1});
        return id;
    };

    map<tuple<int,long long,long long>, int> atomMap;
    map<tuple<int,int,int>, int> hashCons;

    function<int(int,long long,long long)> getAtom = [&](int r, long long lo, long long hi) -> int {
        if (lo > hi) return -1;
        if (r == 0) return END;
        if (lo == 0 && hi == (1LL << r) - 1) return getW(r);
        auto key = make_tuple(r, lo, hi);
        auto it = atomMap.find(key);
        if (it != atomMap.end()) return it->second;

        long long shift = 1LL << (r - 1);
        int c0, c1;
        {
            long long nlo = max(0LL, lo);
            long long nhi = min(shift - 1, hi);
            c0 = (nlo > nhi) ? -1 : getAtom(r - 1, nlo, nhi);
        }
        {
            long long nlo = max(0LL, lo - shift);
            long long nhi = min(shift - 1, hi - shift);
            c1 = (nlo > nhi) ? -1 : getAtom(r - 1, nlo, nhi);
        }

        auto hcKey = make_tuple(r, c0, c1);
        auto it2 = hashCons.find(hcKey);
        int id;
        if (it2 != hashCons.end()) {
            id = it2->second;
        } else {
            id = newNode();
            hashCons[hcKey] = id;
            if (c0 != -1) edges[id].push_back({c0, 0});
            if (c1 != -1) edges[id].push_back({c1, 1});
        }
        atomMap[key] = id;
        return id;
    };

    int START = newNode();

    for (int k = Kmin; k <= Kmax; k++) {
        long long Lk = max(L, 1LL << (k - 1));
        long long Rk = min(R, (1LL << k) - 1);
        if (Lk > Rk) continue;
        long long lo = Lk - (1LL << (k - 1));
        long long hi = Rk - (1LL << (k - 1));
        int t = getAtom(k - 1, lo, hi);
        if (t != -1) edges[START].push_back({t, 1});
    }

    int n = (int)edges.size();
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << edges[i].size();
        for (auto& [a, w] : edges[i]) cout << " " << (a + 1) << " " << w;
        cout << "\n";
    }
    return 0;
}
