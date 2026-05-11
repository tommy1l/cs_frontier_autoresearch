/**
 * Problem 2: Hidden Permutation guess with few queries.
 *
 * Phase 1: find pos_1 via bisection. Query Q_k(S, aux) = [1 at p ∈ S, aux else].
 *   match = 1[pos_1 ∈ S] + 1[pos_aux ∉ S]. Decisive if match ∈ {0, 2}, else
 *   rotate aux and retry. ~2*log2 queries expected.
 * Fallback: if budget exhausted, linear pair-test on remaining range.
 *
 * Phase 2: for each remaining value v, binary-search pos_v using value 1 as
 *   filler. Each query halves candidate set.
 */
#include <bits/stdc++.h>
using namespace std;

static int n;

static int query(const vector<int>& q) {
    cout << 0;
    for (int x : q) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    int a; cin >> a;
    return a;
}

static void answer(const vector<int>& p) {
    cout << 1;
    for (int x : p) cout << ' ' << x;
    cout << '\n';
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    if (n == 1) { answer({1}); return 0; }

    vector<int> perm(n + 1, 0);

    // Phase 1: find pos_1 via bisection with auxiliary fillers.
    int pos_1 = -1;
    {
        int lo = 1, hi = n;
        int aux = 2;
        int budget = 3 * (int)ceil(log2((double)n)) + 5;
        int used = 0;
        while (lo < hi && used < budget) {
            int mid = (lo + hi) / 2;
            vector<int> q(n, aux);
            for (int p = lo; p <= mid; p++) q[p - 1] = 1;
            int a = query(q);
            used++;
            if (a == 2) hi = mid;
            else if (a == 0) lo = mid + 1;
            else {
                aux++;
                if (aux > n) aux = 2;
            }
        }
        if (lo == hi) pos_1 = lo;
    }

    if (pos_1 == -1) {
        // Fallback: try each position via single-shot pair test.
        for (int p = 1; p <= n && pos_1 == -1; p++) {
            vector<int> q(n, 2);
            q[p - 1] = 1;
            int a = query(q);
            if (a == 2) pos_1 = p;
        }
        if (pos_1 == -1) pos_1 = 1; // last resort
    }
    perm[pos_1] = 1;

    // Phase 2: binary search pos_v for each v >= 2.
    vector<int> unknown;
    for (int p = 1; p <= n; p++) if (p != pos_1) unknown.push_back(p);

    for (int v = 2; v <= n; v++) {
        if (unknown.empty()) break;
        if ((int)unknown.size() == 1) {
            perm[unknown[0]] = v;
            unknown.clear();
            continue;
        }
        int known_count = n - (int)unknown.size();
        int lo = 0, hi = (int)unknown.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> q(n, 1);
            for (int p = 1; p <= n; p++) if (perm[p]) q[p - 1] = perm[p];
            for (int k = lo; k <= mid; k++) q[unknown[k] - 1] = v;
            int a = query(q);
            if (a > known_count) hi = mid;
            else lo = mid + 1;
        }
        perm[unknown[lo]] = v;
        unknown.erase(unknown.begin() + lo);
    }

    // Safety.
    if (!unknown.empty()) {
        set<int> used;
        for (int p = 1; p <= n; p++) if (perm[p]) used.insert(perm[p]);
        int v = 1;
        for (int p : unknown) {
            while (used.count(v)) v++;
            perm[p] = v;
            used.insert(v);
        }
    }

    vector<int> ans(n);
    for (int i = 0; i < n; i++) ans[i] = perm[i + 1];
    answer(ans);
    return 0;
}
