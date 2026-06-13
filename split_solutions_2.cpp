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
        int ans = do_query({1, 2});
        if (ans == 2) output_answer({1, 2});
        else output_answer({2, 1});
        return 0;
    }

    vector<int> p(n + 1, 0);

    // ===== Phase 1: joint bit-split BS for pos(1), pos(2) =====
    int log_n = 0;
    while ((1 << log_n) < n) log_n++;

    int kP1 = 0, kP2 = 0, kMask = 0, uMask = 0;
    for (int b = 0; b < log_n; b++) {
        vector<int> q(n, 1);
        for (int j = 1; j <= n; j++) if ((j - 1) >> b & 1) q[j - 1] = 2;
        int ans = do_query(q);
        if (ans == 0) { kP1 |= (1 << b); kMask |= (1 << b); }
        else if (ans == 2) { kP2 |= (1 << b); kMask |= (1 << b); }
        else { uMask |= (1 << b); }
    }
    vector<int> ub;
    for (int b = 0; b < log_n; b++) if (uMask >> b & 1) ub.push_back(b);

    vector<int> candP1, candP2;
    for (int mask = 0; mask < (1 << (int)ub.size()); mask++) {
        int v = 0;
        for (int i = 0; i < (int)ub.size(); i++) if (mask >> i & 1) v |= (1 << ub[i]);
        int p1 = (kP1 | v) + 1, p2 = (kP2 | v) + 1;
        if (p1 >= 1 && p1 <= n && p2 >= 1 && p2 <= n && p1 != p2) {
            candP1.push_back(p1); candP2.push_back(p2);
        }
    }
    int pos1, pos2;
    if (candP1.size() == 1) { pos1 = candP1[0]; pos2 = candP2[0]; }
    else {
        vector<int> sortedP1 = candP1;
        sort(sortedP1.begin(), sortedP1.end());
        sortedP1.erase(unique(sortedP1.begin(), sortedP1.end()), sortedP1.end());

        int lo = 0, hi = (int)sortedP1.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<bool> inS(n + 1, false);
            for (int k = lo; k <= mid; k++) inS[sortedP1[k]] = true;
            vector<int> q(n, 1);
            for (int j = 1; j <= n; j++) if (inS[j]) q[j - 1] = 2;
            int ans = do_query(q);
            if (ans == 0) hi = mid;
            else lo = mid + 1;
        }
        pos1 = sortedP1[lo];
        int v_final = (pos1 - 1) & uMask;
        pos2 = (kP2 | v_final) + 1;
    }
    p[pos1] = 1;
    p[pos2] = 2;

    // ===== Phase 2: triple batch — pair-BS for (v1,v2) + single-BS for v3 =====
    vector<int> U;
    for (int i = 1; i <= n; i++) if (p[i] == 0) U.push_back(i);

    int next_v = 3;
    while (next_v + 1 <= n) {
        int v1 = next_v, v2 = next_v + 1;
        bool has_v3 = (next_v + 2 <= n);
        int v3 = has_v3 ? next_v + 2 : -1;
        next_v += has_v3 ? 3 : 2;

        int m = (int)U.size();
        if (m < 2) break;

        int constMatch = n - m;

        // ----- pair-BS for (v1, v2) among m unknowns -----
        int rp1_final = -1, rp2_final = -1;

        if (m == 2) {
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
            q[U[0] - 1] = v1;
            q[U[1] - 1] = v2;
            int ans = do_query(q);
            int delta = ans - constMatch;
            if (delta == 2) { rp1_final = 0; rp2_final = 1; }
            else { rp1_final = 1; rp2_final = 0; }
        } else {
            int log_m = 0;
            while ((1 << log_m) < m) log_m++;

            int pkP1 = 0, pkP2 = 0, pkMask = 0, puMask = 0;
            for (int b = 0; b < log_m; b++) {
                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
                for (int k = 0; k < m; k++) {
                    q[U[k] - 1] = ((k >> b) & 1) ? v2 : v1;
                }
                int ans = do_query(q);
                int delta = ans - constMatch;
                if (delta == 0) { pkP1 |= (1 << b); pkMask |= (1 << b); }
                else if (delta == 2) { pkP2 |= (1 << b); pkMask |= (1 << b); }
                else { puMask |= (1 << b); }
            }

            vector<int> pub;
            for (int b = 0; b < log_m; b++) if (puMask >> b & 1) pub.push_back(b);

            vector<int> cR1, cR2;
            for (int mask = 0; mask < (1 << (int)pub.size()); mask++) {
                int v = 0;
                for (int i = 0; i < (int)pub.size(); i++) if (mask >> i & 1) v |= (1 << pub[i]);
                int rp1 = pkP1 | v, rp2 = pkP2 | v;
                if (rp1 < m && rp2 < m && rp1 != rp2) {
                    cR1.push_back(rp1); cR2.push_back(rp2);
                }
            }

            if (cR1.size() == 1) {
                rp1_final = cR1[0]; rp2_final = cR2[0];
            } else {
                vector<int> sortedR1 = cR1;
                sort(sortedR1.begin(), sortedR1.end());
                sortedR1.erase(unique(sortedR1.begin(), sortedR1.end()), sortedR1.end());
                int lo = 0, hi = (int)sortedR1.size() - 1;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    vector<bool> inS(m, false);
                    for (int k = lo; k <= mid; k++) inS[sortedR1[k]] = true;
                    vector<int> q(n);
                    for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
                    for (int k = 0; k < m; k++) {
                        q[U[k] - 1] = inS[k] ? v1 : v2;
                    }
                    int ans = do_query(q);
                    int delta = ans - constMatch;
                    if (delta == 2) hi = mid;
                    else lo = mid + 1;
                }
                rp1_final = sortedR1[lo];
                int vf = rp1_final & puMask;
                rp2_final = pkP2 | vf;
            }
        }

        int pos_v1 = U[rp1_final], pos_v2 = U[rp2_final];
        p[pos_v1] = v1; p[pos_v2] = v2;

        vector<int> newU;
        for (int u : U) if (u != pos_v1 && u != pos_v2) newU.push_back(u);
        U = newU;

        // ----- single-BS for v3 among remaining U positions -----
        if (has_v3 && !U.empty()) {
            int mm = (int)U.size();
            if (mm == 1) {
                p[U[0]] = v3;
                U.clear();
            } else {
                int newConst = n - mm;
                int lo = 0, hi = mm - 1;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    vector<int> q(n);
                    for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 1;
                    for (int k = lo; k <= mid; k++) q[U[k] - 1] = v3;
                    int ans = do_query(q);
                    int delta = ans - newConst;
                    if (delta >= 1) hi = mid;
                    else lo = mid + 1;
                }
                p[U[lo]] = v3;
                vector<int> nU;
                for (int i = 0; i < mm; i++) if (i != lo) nU.push_back(U[i]);
                U = nU;
            }
        }
    }

    if (next_v <= n && !U.empty()) {
        p[U[0]] = next_v;
    }

    vector<int> ans_perm;
    for (int i = 1; i <= n; i++) ans_perm.push_back(p[i]);
    output_answer(ans_perm);
    return 0;
}
