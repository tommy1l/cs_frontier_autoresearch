/**
 * Problem 2: Hidden Permutation guess with few queries.
 *
 * Phase 1: find perm[1] via pair queries q=[va, vb, vb, ..., vb].
 *   answer == 2 -> perm[1]=va; == 0 -> perm[1]=vb; == 1 -> neither.
 *   Worst case n/2 queries.
 *
 * Phase 2: for each remaining value v, binary-search pos_v using anchor=perm[1]
 *   as filler. Each query halves candidate set; ~log2 queries per value.
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

    // Phase 1.
    int anchor = -1;
    for (int va = 1; va + 1 <= n; va += 2) {
        int vb = va + 1;
        vector<int> q(n, vb);
        q[0] = va;
        int a = query(q);
        if (a == 2) { anchor = va; break; }
        if (a == 0) { anchor = vb; break; }
    }
    if (anchor == -1) anchor = n; // odd n; perm[1] is the unpaired value
    perm[1] = anchor;

    // Phase 2.
    vector<int> unknown;
    for (int p = 2; p <= n; p++) unknown.push_back(p);

    for (int v = 1; v <= n; v++) {
        if (v == anchor) continue;
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
            vector<int> q(n, anchor);
            for (int p = 1; p <= n; p++) if (perm[p]) q[p - 1] = perm[p];
            for (int k = lo; k <= mid; k++) q[unknown[k] - 1] = v;
            int a = query(q);
            if (a > known_count) hi = mid;
            else lo = mid + 1;
        }
        perm[unknown[lo]] = v;
        unknown.erase(unknown.begin() + lo);
    }

    // Fill any leftover.
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
