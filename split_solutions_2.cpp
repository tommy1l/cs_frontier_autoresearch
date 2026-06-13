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
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
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

    // ===== Phase 1: joint bit-split BS for pos(1), pos(2) (unchanged from pair-bs anchor) =====
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

    // ===== Phase 2: TRIT-JOINT triple BS for (v1, v2, v3) =====
    vector<int> U;
    for (int i = 1; i <= n; i++) if (p[i] == 0) U.push_back(i);

    int next_v = 3;
    while (next_v + 2 <= n) {
        int v1 = next_v, v2 = next_v + 1, v3 = next_v + 2;
        int m = (int)U.size();
        if (m < 3) break;
        next_v += 3;

        int constMatch = n - m;

        int log3_m = 0;
        long long pw = 1;
        while (pw < m) { log3_m++; pw *= 3; }
        if (log3_m == 0) log3_m = 1;

        // a_mask[k] bit t = [trit_t(k)==0]; similarly b (==1), c (==2)
        vector<int> a_mask(m, 0), b_mask(m, 0), c_mask(m, 0);
        for (int k = 0; k < m; k++) {
            long long pp = 1;
            for (int t = 0; t < log3_m; t++) {
                int tt = (k / (int)pp) % 3;
                if (tt == 0) a_mask[k] |= (1 << t);
                else if (tt == 1) b_mask[k] |= (1 << t);
                else c_mask[k] |= (1 << t);
                pp *= 3;
            }
        }

        // log3_m trit-split queries
        vector<int> deltas(log3_m);
        for (int t = 0; t < log3_m; t++) {
            long long pp = 1; for (int i = 0; i < t; i++) pp *= 3;
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
            for (int k = 0; k < m; k++) {
                int tt = (k / (int)pp) % 3;
                int val = (tt == 0) ? v1 : (tt == 1) ? v2 : v3;
                q[U[k] - 1] = val;
            }
            int ans = do_query(q);
            deltas[t] = ans - constMatch;
        }

        // Group positions by c_mask for fast k3 lookup
        vector<vector<int>> c_to_pos(1 << log3_m);
        for (int k = 0; k < m; k++) c_to_pos[c_mask[k]].push_back(k);

        // Enumerate candidate triples (k1, k2, k3)
        vector<tuple<int, int, int>> cands;
        for (int k1 = 0; k1 < m; k1++) {
            int am = a_mask[k1];
            for (int k2 = 0; k2 < m; k2++) {
                if (k2 == k1) continue;
                int bm = b_mask[k2];
                int target_c = 0;
                bool feasible = true;
                for (int t = 0; t < log3_m; t++) {
                    int a_t = (am >> t) & 1;
                    int b_t = (bm >> t) & 1;
                    int req = deltas[t] - a_t - b_t;
                    if (req < 0 || req > 1) { feasible = false; break; }
                    if (req == 1) target_c |= (1 << t);
                }
                if (!feasible) continue;
                for (int k3 : c_to_pos[target_c]) {
                    if (k3 == k1 || k3 == k2) continue;
                    cands.push_back(make_tuple(k1, k2, k3));
                }
            }
        }

        int rk1 = -1, rk2 = -1, rk3 = -1;
        if (cands.size() == 1) {
            tie(rk1, rk2, rk3) = cands[0];
        } else if (!cands.empty()) {
            vector<tuple<int, int, int>> rem = cands;
            int safety = 50;
            while (rem.size() > 1 && safety-- > 0) {
                int tk1, tk2, tk3;
                tie(tk1, tk2, tk3) = rem[0];
                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
                q[U[tk1] - 1] = v1; q[U[tk2] - 1] = v2; q[U[tk3] - 1] = v3;
                int ans = do_query(q);
                int actual = ans - constMatch;
                vector<tuple<int, int, int>> next_rem;
                for (auto& tup : rem) {
                    int ck1, ck2, ck3;
                    tie(ck1, ck2, ck3) = tup;
                    int pd = (ck1 == tk1) + (ck2 == tk2) + (ck3 == tk3);
                    if (pd == actual) next_rem.push_back(tup);
                }
                if (next_rem.empty()) break;
                rem = next_rem;
            }
            if (!rem.empty()) tie(rk1, rk2, rk3) = rem[0];
        }

        if (rk1 < 0) {
            // emergency fallback (shouldn't happen)
            rk1 = 0; rk2 = 1; rk3 = 2;
        }

        p[U[rk1]] = v1;
        p[U[rk2]] = v2;
        p[U[rk3]] = v3;

        vector<int> newU;
        for (int k = 0; k < m; k++) if (k != rk1 && k != rk2 && k != rk3) newU.push_back(U[k]);
        U = newU;
    }

    // Leftover: 1 or 2 values via simple BS
    while (next_v + 1 <= n) {
        int v1 = next_v, v2 = next_v + 1;
        next_v += 2;
        int m = (int)U.size();
        if (m < 2) break;
        int constMatch = n - m;
        if (m == 2) {
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
            q[U[0] - 1] = v1; q[U[1] - 1] = v2;
            int ans = do_query(q);
            int d = ans - constMatch;
            if (d == 2) { p[U[0]] = v1; p[U[1]] = v2; }
            else { p[U[0]] = v2; p[U[1]] = v1; }
            U.clear();
            continue;
        }
        // single-BS for v1, then v2
        int lo = 0, hi = m - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 1;
            for (int k = lo; k <= mid; k++) q[U[k] - 1] = v1;
            int ans = do_query(q);
            if (ans - (n - m) >= 1) hi = mid;
            else lo = mid + 1;
        }
        int pv1 = U[lo];
        p[pv1] = v1;
        vector<int> nU;
        for (int u : U) if (u != pv1) nU.push_back(u);
        U = nU;
        int mm = (int)U.size();
        if (mm == 0) break;
        if (mm == 1) { p[U[0]] = v2; U.clear(); break; }
        int lo2 = 0, hi2 = mm - 1;
        while (lo2 < hi2) {
            int mid = (lo2 + hi2) / 2;
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 1;
            for (int k = lo2; k <= mid; k++) q[U[k] - 1] = v2;
            int ans = do_query(q);
            if (ans - (n - mm) >= 1) hi2 = mid;
            else lo2 = mid + 1;
        }
        int pv2 = U[lo2];
        p[pv2] = v2;
        vector<int> nU2;
        for (int u : U) if (u != pv2) nU2.push_back(u);
        U = nU2;
    }

    if (next_v <= n && !U.empty()) {
        p[U[0]] = next_v;
    }

    vector<int> ans_perm;
    for (int i = 1; i <= n; i++) ans_perm.push_back(p[i]);
    output_answer(ans_perm);
    return 0;
}
