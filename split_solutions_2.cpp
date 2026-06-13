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

    // ===== Phase 2: TRIO-JOINT (m >= 16) -> pair-BS -> tail-brute =====
    vector<int> U;
    for (int i = 1; i <= n; i++) if (p[i] == 0) U.push_back(i);

    const int TAIL = 8;
    const int TRIO_MIN = 16;
    int next_v = 3;

    while (next_v <= n && !U.empty()) {
        int m = (int)U.size();

        if (m <= TAIL) {
            // ===== Tail-brute (existing) =====
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

        // ===== Try TRIO-JOINT for m >= 16 and >= 3 values left =====
        bool used_trio = false;
        if (m >= TRIO_MIN && next_v + 2 <= n) {
            int v0 = next_v, v1 = next_v + 1, v2 = next_v + 2;

            int log2m = 0;
            while ((1 << log2m) < m) log2m++;
            int T_q = (int)ceil(1.5 * (double)log2m) + 3;

            // Build trit matrix M[t][idx]
            int Tbase = 0;
            { long long p3v = 1; while (p3v < m) { Tbase++; p3v *= 3; } }

            vector<vector<int>> M(T_q, vector<int>(m));
            long long pp3 = 1;
            for (int t = 0; t < min(Tbase, T_q); t++) {
                for (int idx = 0; idx < m; idx++) {
                    M[t][idx] = (int)((idx / pp3) % 3);
                }
                pp3 *= 3;
            }
            for (int t = Tbase; t < T_q; t++) {
                unsigned int seed = (unsigned int)(t * 2654435761u) ^ 0xa5a5a5a5u;
                for (int idx = 0; idx < m; idx++) {
                    unsigned int h = (unsigned int)idx * 2246822519u + seed;
                    h ^= (h >> 13);
                    h *= 3266489917u;
                    h ^= (h >> 16);
                    M[t][idx] = (int)(h % 3u);
                }
            }

            // Send queries
            int constMatch = n - m;
            vector<int> r(T_q);
            for (int t = 0; t < T_q; t++) {
                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : v0;
                for (int idx = 0; idx < m; idx++) {
                    int d = M[t][idx];
                    int v = (d == 0) ? v0 : (d == 1) ? v1 : v2;
                    q[U[idx] - 1] = v;
                }
                int ans = do_query(q);
                r[t] = ans - constMatch;
            }

            // Decode: decisive per-value filter, then joint enum
            vector<vector<int>> C(3);
            for (int j = 0; j < 3; j++) {
                C[j].reserve(m);
                for (int idx = 0; idx < m; idx++) C[j].push_back(idx);
            }

            bool decode_ok = true;
            for (int t = 0; t < T_q && decode_ok; t++) {
                if (r[t] == 0) {
                    for (int j = 0; j < 3; j++) {
                        vector<int> newC;
                        for (int idx : C[j]) if (M[t][idx] != j) newC.push_back(idx);
                        C[j] = move(newC);
                        if (C[j].empty()) { decode_ok = false; break; }
                    }
                } else if (r[t] == 3) {
                    for (int j = 0; j < 3; j++) {
                        vector<int> newC;
                        for (int idx : C[j]) if (M[t][idx] == j) newC.push_back(idx);
                        C[j] = move(newC);
                        if (C[j].empty()) { decode_ok = false; break; }
                    }
                }
            }

            const long long ENUM_LIMIT = 5000000;
            long long total = (long long)C[0].size() * (long long)C[1].size() * (long long)C[2].size();

            // Adaptive extension: if enum input too large, send more queries
            // and tighten C[j] via decisive r=0/3 filter, avoiding the costly
            // fall-through to pair-BS (which wastes the T_q queries spent here).
            int T_max_ext = T_q + 10;
            while (decode_ok && total > ENUM_LIMIT && T_q < T_max_ext) {
                int t = T_q;
                M.push_back(vector<int>(m));
                unsigned int seed = (unsigned int)(t * 2654435761u) ^ 0xa5a5a5a5u;
                for (int idx = 0; idx < m; idx++) {
                    unsigned int h = (unsigned int)idx * 2246822519u + seed;
                    h ^= (h >> 13);
                    h *= 3266489917u;
                    h ^= (h >> 16);
                    M[t][idx] = (int)(h % 3u);
                }

                vector<int> q(n);
                for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : v0;
                for (int idx = 0; idx < m; idx++) {
                    int d = M[t][idx];
                    int v = (d == 0) ? v0 : (d == 1) ? v1 : v2;
                    q[U[idx] - 1] = v;
                }
                int ans = do_query(q);
                int rt = ans - constMatch;
                r.push_back(rt);

                if (rt == 0) {
                    for (int j = 0; j < 3; j++) {
                        vector<int> newC;
                        for (int idx : C[j]) if (M[t][idx] != j) newC.push_back(idx);
                        C[j] = move(newC);
                        if (C[j].empty()) { decode_ok = false; break; }
                    }
                } else if (rt == 3) {
                    for (int j = 0; j < 3; j++) {
                        vector<int> newC;
                        for (int idx : C[j]) if (M[t][idx] == j) newC.push_back(idx);
                        C[j] = move(newC);
                        if (C[j].empty()) { decode_ok = false; break; }
                    }
                }

                T_q++;
                total = (long long)C[0].size() * (long long)C[1].size() * (long long)C[2].size();
            }

            if (decode_ok && total > 0 && total <= ENUM_LIMIT) {
                vector<tuple<int,int,int>> triples;
                int trip_cap = 200000;
                bool overflow = false;
                for (int i0 : C[0]) {
                    if (overflow) break;
                    for (int i1 : C[1]) {
                        if (i1 == i0) continue;
                        if (overflow) break;
                        for (int i2 : C[2]) {
                            if (i2 == i0 || i2 == i1) continue;
                            bool ok = true;
                            for (int t = 0; t < T_q; t++) {
                                if (r[t] == 0 || r[t] == 3) continue;
                                int s = (M[t][i0] == 0) + (M[t][i1] == 1) + (M[t][i2] == 2);
                                if (s != r[t]) { ok = false; break; }
                            }
                            if (ok) {
                                triples.push_back({i0, i1, i2});
                                if ((int)triples.size() > trip_cap) { overflow = true; break; }
                            }
                        }
                    }
                }

                if (!triples.empty() && !overflow) {
                    // Cand-elim if multiple
                    while (triples.size() > 1) {
                        auto [a, b, c] = triples[0];
                        vector<int> q(n);
                        for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 1;
                        q[U[a] - 1] = v0;
                        q[U[b] - 1] = v1;
                        q[U[c] - 1] = v2;
                        int ans = do_query(q);
                        int delta = ans - constMatch;

                        vector<tuple<int,int,int>> newT;
                        for (auto& tup : triples) {
                            auto [aa, bb, cc] = tup;
                            int s = (aa == a) + (bb == b) + (cc == c);
                            if (s == delta) newT.push_back(tup);
                        }
                        triples = move(newT);
                        if (triples.empty()) break;
                    }

                    if (!triples.empty()) {
                        auto [wa, wb, wc] = triples[0];
                        p[U[wa]] = v0; p[U[wb]] = v1; p[U[wc]] = v2;

                        vector<int> newU;
                        for (int idx = 0; idx < m; idx++) {
                            if (idx != wa && idx != wb && idx != wc) newU.push_back(U[idx]);
                        }
                        U = move(newU);
                        next_v += 3;
                        used_trio = true;
                    }
                }
            }

            // ===== C-set BS fallback (alternative decoder on spent trio queries) =====
            // When enum was infeasible (total > ENUM_LIMIT or empty triples or overflow),
            // recover all 3 values by single-value BS on each C[j] instead of wasting
            // the T_q packed queries and restarting via pair-BS.
            if (!used_trio && decode_ok && !C[0].empty() && !C[1].empty() && !C[2].empty()) {
                int v_vals[3] = {v0, v1, v2};
                int found_idx[3] = {-1, -1, -1};
                bool cset_ok = true;

                for (int j = 0; j < 3 && cset_ok; j++) {
                    vector<int> cand_idx;
                    cand_idx.reserve(C[j].size());
                    for (int idx : C[j]) {
                        bool skip = false;
                        for (int k = 0; k < j; k++) if (idx == found_idx[k]) { skip = true; break; }
                        if (!skip) cand_idx.push_back(idx);
                    }
                    if (cand_idx.empty()) { cset_ok = false; break; }
                    sort(cand_idx.begin(), cand_idx.end());

                    int lo = 0, hi = (int)cand_idx.size() - 1;
                    while (lo < hi) {
                        int mid = (lo + hi) / 2;
                        vector<int> q(n);
                        for (int i = 1; i <= n; i++) q[i - 1] = (p[i] != 0) ? p[i] : 1;
                        for (int k = lo; k <= mid; k++) {
                            q[U[cand_idx[k]] - 1] = v_vals[j];
                        }
                        int ans = do_query(q);
                        int delta = ans - constMatch;
                        if (delta >= 1) hi = mid;
                        else lo = mid + 1;
                    }
                    found_idx[j] = cand_idx[lo];
                }

                if (cset_ok) {
                    p[U[found_idx[0]]] = v0;
                    p[U[found_idx[1]]] = v1;
                    p[U[found_idx[2]]] = v2;
                    vector<int> newU;
                    for (int idx = 0; idx < m; idx++) {
                        if (idx != found_idx[0] && idx != found_idx[1] && idx != found_idx[2])
                            newU.push_back(U[idx]);
                    }
                    U = move(newU);
                    next_v += 3;
                    used_trio = true;
                }
            }
        }

        if (used_trio) continue;

        // ===== Pair-BS fallback (from 29aacae) =====
        int v1 = next_v, v2 = next_v + 1;
        next_v += 2;
        if (m < 2) break;

        int constMatch = n - m;

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
