/**
 * Problem 2: Hidden Permutation. Paired bisection in phase 2, including joint
 * post-separation bisection (1 query halves BOTH disjoint candidate sets when
 * decisive, +1 disambig query otherwise).
 */
#include <bits/stdc++.h>
using namespace std;

static int n;
static int anchor_val;
static vector<int> perm_global;

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

static vector<int> baseQ() {
    vector<int> q(n, anchor_val);
    for (int p = 1; p <= n; p++) if (perm_global[p]) q[p - 1] = perm_global[p];
    return q;
}

static void markFound(int v, int p, vector<int>& unknown) {
    perm_global[p] = v;
    auto it = find(unknown.begin(), unknown.end(), p);
    if (it != unknown.end()) unknown.erase(it);
}

static void searchSingle(int v, vector<int> cand, vector<int>& unknown) {
    while (cand.size() > 1) {
        int half = (int)cand.size() / 2;
        vector<int> q = baseQ();
        for (int k = 0; k < half; k++) q[cand[k] - 1] = v;
        int a = query(q);
        int known_count = n - (int)unknown.size();
        if (a > known_count) cand.resize(half);
        else cand.erase(cand.begin(), cand.begin() + half);
    }
    markFound(v, cand[0], unknown);
}

static void searchPairDisjoint(int v1, int v2,
                                vector<int> c1, vector<int> c2,
                                vector<int>& unknown) {
    while ((int)c1.size() > 1 && (int)c2.size() > 1) {
        int sz1 = (int)c1.size(), sz2 = (int)c2.size();
        int h1 = sz1 / 2, h2 = sz2 / 2;
        vector<int> q = baseQ();
        for (int k = 0; k < h1; k++) q[c1[k] - 1] = v1;
        for (int k = 0; k < h2; k++) q[c2[k] - 1] = v2;
        int a = query(q);
        int known_count = n - (int)unknown.size();
        int delta = a - known_count;
        if (delta == 2) {
            c1.resize(h1);
            c2.resize(h2);
        } else if (delta == 0) {
            c1.erase(c1.begin(), c1.begin() + h1);
            c2.erase(c2.begin(), c2.begin() + h2);
        } else {
            // delta == 1: ambiguous; disambig by testing only v1's left.
            vector<int> q2 = baseQ();
            for (int k = 0; k < h1; k++) q2[c1[k] - 1] = v1;
            int a2 = query(q2);
            if (a2 > known_count) {
                c1.resize(h1);
                c2.erase(c2.begin(), c2.begin() + h2);
            } else {
                c1.erase(c1.begin(), c1.begin() + h1);
                c2.resize(h2);
            }
        }
    }
    if ((int)c1.size() == 1) markFound(v1, c1[0], unknown);
    else searchSingle(v1, c1, unknown);
    if ((int)c2.size() == 1) markFound(v2, c2[0], unknown);
    else searchSingle(v2, c2, unknown);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    if (n == 1) { answer({1}); return 0; }

    perm_global.assign(n + 1, 0);

    // Phase 1: linear pair-test scan.
    int anchor = -1;
    for (int va = 1; va + 1 <= n; va += 2) {
        int vb = va + 1;
        vector<int> q(n, vb);
        q[0] = va;
        int a = query(q);
        if (a == 2) { anchor = va; break; }
        if (a == 0) { anchor = vb; break; }
    }
    if (anchor == -1) anchor = n;
    perm_global[1] = anchor;
    anchor_val = anchor;

    vector<int> remaining;
    for (int v = 1; v <= n; v++) if (v != anchor) remaining.push_back(v);
    vector<int> unknown;
    for (int p = 2; p <= n; p++) unknown.push_back(p);

    int i = 0;
    while (i < (int)remaining.size()) {
        if (unknown.empty()) break;
        int v1 = remaining[i];
        if ((int)unknown.size() == 1) {
            markFound(v1, unknown[0], unknown);
            i++;
            continue;
        }
        if (i + 1 >= (int)remaining.size()) {
            searchSingle(v1, unknown, unknown);
            i++;
            continue;
        }
        int v2 = remaining[i + 1];

        // Joint same-set bisection until separation.
        vector<int> cand_common = unknown;
        vector<int> cand_v1, cand_v2;
        bool separated = false;
        while (!separated && (int)cand_common.size() > 1) {
            int sz = (int)cand_common.size();
            int half = (sz + 1) / 2; // ceil split
            vector<int> left(cand_common.begin(), cand_common.begin() + half);
            vector<int> right(cand_common.begin() + half, cand_common.end());
            vector<int> q = baseQ();
            for (int p : left) q[p - 1] = v1;
            for (int p : right) q[p - 1] = v2;
            int a = query(q);
            int known_count = n - (int)unknown.size();
            int delta = a - known_count;
            if (delta == 2) { cand_v1 = left; cand_v2 = right; separated = true; }
            else if (delta == 0) { cand_v1 = right; cand_v2 = left; separated = true; }
            else {
                // delta == 1: same side; disambiguate.
                vector<int> q2 = baseQ();
                for (int p : left) q2[p - 1] = v1;
                int a2 = query(q2);
                if (a2 > known_count) cand_common = left;
                else cand_common = right;
            }
        }

        if (separated) {
            searchPairDisjoint(v1, v2, cand_v1, cand_v2, unknown);
            i += 2;
        } else {
            markFound(v1, cand_common[0], unknown);
            i++;
        }
    }

    if (!unknown.empty()) {
        set<int> used;
        for (int p = 1; p <= n; p++) if (perm_global[p]) used.insert(perm_global[p]);
        int v = 1;
        for (int p : unknown) {
            while (used.count(v)) v++;
            perm_global[p] = v;
            used.insert(v);
        }
    }

    vector<int> ans(n);
    for (int i = 0; i < n; i++) ans[i] = perm_global[i + 1];
    answer(ans);
    return 0;
}
