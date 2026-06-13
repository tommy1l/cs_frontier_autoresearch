#include <bits/stdc++.h>
using namespace std;

int n;

int do_query(const vector<int>& q) {
    cout << 0;
    for (int x : q) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    int x; cin >> x;
    return x;
}

void output_answer(const vector<int>& p) {
    cout << 1;
    for (int x : p) cout << ' ' << x;
    cout << '\n';
    cout.flush();
}

int main() {
    cin >> n;

    if (n == 1) {
        output_answer({1});
        return 0;
    }

    if (n == 2) {
        vector<int> q = {1, 2};
        int ans = do_query(q);
        if (ans == 2) output_answer({1, 2});
        else output_answer({2, 1});
        return 0;
    }

    vector<int> p(n + 1, 0);

    // Phase 1a: bit-split queries to jointly constrain (pos_of_1, pos_of_2).
    // For bit b, query Q[j] = 2 if (j-1) has bit b set, else 1.
    // ans = [bit_b(pos_2-1)=1] + [bit_b(pos_1-1)=0]
    //   ans=0: bit_b(p1-1)=1, bit_b(p2-1)=0
    //   ans=2: bit_b(p1-1)=0, bit_b(p2-1)=1
    //   ans=1: equal bits (same)
    int log_n = 0;
    while ((1 << log_n) < n) log_n++;

    int knownP1 = 0, knownP2 = 0;
    int knownMask = 0, unknownMask = 0;

    for (int b = 0; b < log_n; b++) {
        vector<int> q(n, 1);
        for (int j = 1; j <= n; j++) if ((j-1) >> b & 1) q[j-1] = 2;
        int ans = do_query(q);
        if (ans == 0) {
            knownP1 |= (1 << b);
            knownMask |= (1 << b);
        } else if (ans == 2) {
            knownP2 |= (1 << b);
            knownMask |= (1 << b);
        } else {
            unknownMask |= (1 << b);
        }
    }

    // Phase 1b: enumerate valid shared "same-bit" values v, BS over candidates.
    vector<int> ubits;
    for (int b = 0; b < log_n; b++) if (unknownMask >> b & 1) ubits.push_back(b);
    int t = (int)ubits.size();

    vector<int> candVals;
    for (int mask = 0; mask < (1 << t); mask++) {
        int v = 0;
        for (int i = 0; i < t; i++) if (mask >> i & 1) v |= (1 << ubits[i]);
        int p1 = (knownP1 | v) + 1;
        int p2 = (knownP2 | v) + 1;
        if (p1 >= 1 && p1 <= n && p2 >= 1 && p2 <= n && p1 != p2) {
            candVals.push_back(v);
        }
    }

    int pos_of_1, pos_of_2;
    if (candVals.size() == 1) {
        pos_of_1 = (knownP1 | candVals[0]) + 1;
        pos_of_2 = (knownP2 | candVals[0]) + 1;
    } else {
        // BS for pos_of_1 within candidate set.
        // candP1s ∩ candP2s = ∅ when knownMask non-empty (proven), so ans ∈ {0, 1}.
        vector<int> candP1Vec;
        for (int v : candVals) candP1Vec.push_back((knownP1 | v) + 1);
        sort(candP1Vec.begin(), candP1Vec.end());

        int lo = 0, hi = (int)candP1Vec.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<bool> inS(n + 1, false);
            for (int k = lo; k <= mid; k++) inS[candP1Vec[k]] = true;

            vector<int> q(n, 1);
            for (int j = 1; j <= n; j++) if (inS[j]) q[j-1] = 2;
            int ans = do_query(q);

            // ans = [pos_1 ∉ S] + [pos_2 ∈ S]; pos_2 ∉ S by disjointness.
            // ans = 0: pos_1 ∈ S → narrow to lo..mid.
            // ans = 1 (or 2 if disjoint claim wrong): narrow to mid+1..hi.
            if (ans == 0) hi = mid;
            else lo = mid + 1;
        }
        pos_of_1 = candP1Vec[lo];
        int v_final = (pos_of_1 - 1) & unknownMask;
        pos_of_2 = (knownP2 | v_final) + 1;
    }

    p[pos_of_1] = 1;
    p[pos_of_2] = 2;

    vector<int> unknown_positions;
    for (int i = 1; i <= n; i++) if (p[i] == 0) unknown_positions.push_back(i);

    // Phase 2: For each value v = 3..n-1, BS within unknown_positions.
    // Query Q[j] = v for j in T, Q[j] = 1 for j not in T.
    // - pos_of_1: Q=1, p=1, +1 match.
    // - pos_of_v: +1 if pos_of_v in T.
    // - others: no match.
    // Total = 1 + [pos_of_v in T].
    for (int v = 3; v <= n - 1; v++) {
        int lo = 0, hi = (int)unknown_positions.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> q(n, 1);
            for (int k = lo; k <= mid; k++) q[unknown_positions[k] - 1] = v;
            int ans = do_query(q);
            if (ans == 2) hi = mid;
            else lo = mid + 1;
        }
        p[unknown_positions[lo]] = v;
        unknown_positions.erase(unknown_positions.begin() + lo);
    }

    if (!unknown_positions.empty()) {
        p[unknown_positions[0]] = n;
    }

    vector<int> ans_perm;
    for (int i = 1; i <= n; i++) ans_perm.push_back(p[i]);
    output_answer(ans_perm);

    return 0;
}
