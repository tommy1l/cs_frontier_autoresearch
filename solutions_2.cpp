/**
 * Problem 2: Hidden Permutation guess with few queries.
 *
 * Phase 1: find perm[1] via pair queries [va, vb, ..., vb] (linear).
 * Phase 2 (paired): bisect two values v1, v2 simultaneously.
 *   Query q[left] = v1, q[right] = v2, q[else] = anchor.
 *   delta = match - known ∈ {0, 1, 2}.
 *   delta=2 → pos_v1∈left, pos_v2∈right (decisive).
 *   delta=0 → pos_v1∈right, pos_v2∈left (decisive).
 *   delta=1 → same side; disambig with q[left]=v1, rest=anchor.
 *   Saves ~1 query per pair vs sequential bisection.
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
    int p = cand[0];
    perm_global[p] = v;
    auto it = find(unknown.begin(), unknown.end(), p);
    if (it != unknown.end()) unknown.erase(it);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    if (n == 1) { answer({1}); return 0; }

    perm_global.assign(n + 1, 0);

    // Phase 1: pair-test linear scan.
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

    // Phase 2: paired bisection.
    vector<int> remaining;
    for (int v = 1; v <= n; v++) if (v != anchor) remaining.push_back(v);
    vector<int> unknown;
    for (int p = 2; p <= n; p++) unknown.push_back(p);

    int i = 0;
    while (i < (int)remaining.size()) {
        if (unknown.empty()) break;
        int v1 = remaining[i];
        if ((int)unknown.size() == 1) {
            perm_global[unknown[0]] = v1;
            unknown.clear();
            i++;
            continue;
        }
        if (i + 1 >= (int)remaining.size()) {
            searchSingle(v1, unknown, unknown);
            i++;
            continue;
        }
        int v2 = remaining[i + 1];

        vector<int> cand_common = unknown;
        vector<int> cand_v1, cand_v2;
        bool separated = false;

        while (!separated && (int)cand_common.size() > 1) {
            int sz = (int)cand_common.size();
            int half = sz / 2;
            vector<int> left(cand_common.begin(), cand_common.begin() + half);
            vector<int> right(cand_common.begin() + half, cand_common.end());
            vector<int> q = baseQ();
            for (int p : left) q[p - 1] = v1;
            for (int p : right) q[p - 1] = v2;
            int a = query(q);
            int known_count = n - (int)unknown.size();
            int delta = a - known_count;
            if (delta == 2) {
                cand_v1 = left;
                cand_v2 = right;
                separated = true;
            } else if (delta == 0) {
                cand_v1 = right;
                cand_v2 = left;
                separated = true;
            } else {
                // delta == 1: same side. Disambiguate.
                vector<int> q2 = baseQ();
                for (int p : left) q2[p - 1] = v1;
                int a2 = query(q2);
                if (a2 > known_count) cand_common = left;
                else cand_common = right;
            }
        }

        if (separated) {
            searchSingle(v1, cand_v1, unknown);
            searchSingle(v2, cand_v2, unknown);
            i += 2;
        } else {
            // cand_common.size() == 1: both v1 and v2 should be here? Impossible.
            // Fallback: assign v1 here, then search v2 normally.
            perm_global[cand_common[0]] = v1;
            auto it = find(unknown.begin(), unknown.end(), cand_common[0]);
            if (it != unknown.end()) unknown.erase(it);
            i++;
        }
    }

    // Fill any leftover.
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
