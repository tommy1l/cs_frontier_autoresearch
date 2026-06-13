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

    // ===== Phase 2: TRIO joint BS (per-value cand filter, NO full mask enum), pair-BS fallback =====
    vector<int> U;
    for (int i = 1; i <= n; i++) if (p[i] == 0) U.push_back(i);

    const int TAIL = 8;
    int next_v = 3;

    while (next_v <= n && !U.empty()) {
        int m = (int)U.size();

        if (m <= TAIL) {
            // Brute-force tail: candidate-elimination over all m! permutations.
            vector<int> V;
            for (int i = 0; i < m; i++) V.push_back(next_v + i);

            vector<int> perm(m);
            iota(perm.begin(), perm.end(), 0);
            vector<vector<int>> cands;
            do {
                vector<int> c(m);
                for (int i = 0; i < m; i++) c[i] = V[perm[i]];
                cands.push_back(c);
            } while (next_permutation(perm.begin(), perm.end()));

            while (cands.size() > 1) {
                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : V[0];
                for (int k = 0; k < m; k++) q[U[k] - 1] = cands[0][k];
                int ans = do_query(q);
                int delta = ans - (n - m);

                vector<vector<int>> newC;
                for (auto& c : cands) {
                    int matches = 0;
                    for (int k = 0; k < m; k++) if (c[k] == cands[0][k]) matches++;
                    if (matches == delta) newC.push_back(c);
                }
                cands = move(newC);
            }

            for (int k = 0; k < m; k++) p[U[k]] = cands[0][k];
            next_v += m;
            U.clear();
            break;
        }

        int constMatch = n - m;
        bool try_trio = (next_v + 2 <= n);

        bool trio_success = false;
        int found_i1 = -1, found_i2 = -1, found_i3 = -1;

        if (try_trio) {
            int v1 = next_v, v2 = next_v + 1, v3 = next_v + 2;

            // T = ceil(log_3(m)), with cap
            int T = 0; long long p3 = 1;
            while (p3 < m) { T++; p3 *= 3; }

            // precompute powers of 3
            vector<long long> pow3(T + 1, 1);
            for (int t = 1; t <= T; t++) pow3[t] = pow3[t - 1] * 3;

            vector<int> tr_ans(T);
            for (int t = 0; t < T; t++) {
                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
                long long p3t = pow3[t];
                for (int j = 0; j < m; j++) {
                    int d = (int)((j / p3t) % 3);
                    q[U[j] - 1] = (d == 0) ? v1 : (d == 1) ? v2 : v3;
                }
                int a = do_query(q);
                tr_ans[t] = a - constMatch;
            }

            // Per-value cand filtering using DECISIVE trits (ans = 0 or 3)
            vector<vector<int>> CAND(3);
            for (int i = 0; i < 3; i++) {
                for (int idx = 0; idx < m; idx++) {
                    bool ok = true;
                    for (int t = 0; t < T; t++) {
                        if (tr_ans[t] != 0 && tr_ans[t] != 3) continue;
                        int d = (int)((idx / pow3[t]) % 3);
                        if (tr_ans[t] == 0 && d == i) { ok = false; break; }
                        if (tr_ans[t] == 3 && d != i) { ok = false; break; }
                    }
                    if (ok) CAND[i].push_back(idx);
                }
            }

            long long joint_size = (long long)CAND[0].size() *
                                   (long long)CAND[1].size() *
                                   (long long)CAND[2].size();

            if (joint_size > 0 && joint_size <= 300000) {
                int sol_count = 0;
                for (int i1 : CAND[0]) {
                    if (sol_count > 1) break;
                    for (int i2 : CAND[1]) {
                        if (sol_count > 1) break;
                        if (i2 == i1) continue;
                        for (int i3 : CAND[2]) {
                            if (i3 == i1 || i3 == i2) continue;
                            bool ok = true;
                            for (int t = 0; t < T; t++) {
                                int a = tr_ans[t];
                                if (a == 0 || a == 3) continue;
                                int d1 = (int)((i1 / pow3[t]) % 3);
                                int d2 = (int)((i2 / pow3[t]) % 3);
                                int d3 = (int)((i3 / pow3[t]) % 3);
                                int s = (d1 == 0) + (d2 == 1) + (d3 == 2);
                                if (s != a) { ok = false; break; }
                            }
                            if (ok) {
                                found_i1 = i1; found_i2 = i2; found_i3 = i3;
                                sol_count++;
                                if (sol_count > 1) break;
                            }
                        }
                    }
                }
                if (sol_count == 1) trio_success = true;
            }

            if (trio_success) {
                p[U[found_i1]] = v1;
                p[U[found_i2]] = v2;
                p[U[found_i3]] = v3;
                vector<int> newU;
                for (int k = 0; k < m; k++) {
                    if (k != found_i1 && k != found_i2 && k != found_i3) newU.push_back(U[k]);
                }
                U = newU;
                next_v += 3;
                continue;
            }
            // else: fall through to pair-BS for (v1, v2); trio queries are "wasted"
        }

        // ===== Pair-BS fallback for current next_v, next_v+1 =====
        int v1 = next_v, v2 = next_v + 1;
        next_v += 2;
        if (m < 2) break;

        if (m == 2) {
            vector<int> q(n);
            for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 0;
            q[U[0] - 1] = v1;
            q[U[1] - 1] = v2;
            int ans = do_query(q);
            int delta = ans - constMatch;
            if (delta == 2) { p[U[0]] = v1; p[U[1]] = v2; }
            else { p[U[0]] = v2; p[U[1]] = v1; }
            U.clear();
            continue;
        }

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

        int rp1_final, rp2_final;
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

        int pos_v1 = U[rp1_final], pos_v2 = U[rp2_final];
        p[pos_v1] = v1; p[pos_v2] = v2;

        vector<int> newU;
        for (int u : U) if (u != pos_v1 && u != pos_v2) newU.push_back(u);
        U = newU;
    }

    if (next_v <= n && !U.empty()) {
        p[U[0]] = next_v;
    }

    vector<int> ans_perm;
    for (int i = 1; i <= n; i++) ans_perm.push_back(p[i]);
    output_answer(ans_perm);
    return 0;
}
