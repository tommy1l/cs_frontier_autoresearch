#include <cstdio>
#include <vector>
#include <cstring>
#include <algorithm>
using namespace std;

static int N;
static vector<int> RESP;

static void send_query(const vector<int>& toks) {
    printf("%d", (int)toks.size());
    for (int t : toks) printf(" %d", t);
    printf("\n");
    fflush(stdout);
    RESP.assign(toks.size(), 0);
    for (size_t i = 0; i < toks.size(); i++) scanf("%d", &RESP[i]);
}

int main() {
    int subtask;
    scanf("%d %d", &subtask, &N);
    int n = N;
    
    if (n <= 1000) {
        long long L = 2LL * n * (n - 1);
        printf("%lld", L);
        for (int u = 1; u <= n; u++) {
            for (int v = u + 1; v <= n; v++) {
                printf(" %d %d %d %d", u, v, v, u);
            }
        }
        printf("\n");
        fflush(stdout);
        
        vector<vector<int>> neighbors(n + 1);
        
        long long numPairs = (long long)n * (n - 1) / 2;
        int u = 1, v = 2;
        for (long long k = 0; k < numPairs; k++) {
            int r0, r1, r2, r3;
            scanf("%d %d %d %d", &r0, &r1, &r2, &r3);
            if (r1 == 1) {
                neighbors[u].push_back(v);
                neighbors[v].push_back(u);
            }
            v++;
            if (v > n) { u++; v = u + 1; }
        }
        
        printf("-1");
        int prev = 0, cur = 1;
        for (int i = 0; i < n; i++) {
            printf(" %d", cur);
            int nxt;
            if (neighbors[cur][0] != prev) nxt = neighbors[cur][0];
            else nxt = neighbors[cur][1];
            prev = cur;
            cur = nxt;
        }
        printf("\n");
        fflush(stdout);
        return 0;
    }
    
    // n > 1000: full rewrite
    vector<int> A_list;
    vector<char> inA(n + 1, 0);
    
    const int BATCH = 8192;
    for (int start = 1; start <= n; start += BATCH) {
        int end = min(n, start + BATCH - 1);
        vector<int> batch;
        for (int x = start; x <= end; x++) batch.push_back(x);
        
        // Probe query: for each x in batch emit x x
        vector<int> q1;
        q1.reserve(batch.size() * 2);
        for (int x : batch) { q1.push_back(x); q1.push_back(x); }
        send_query(q1);
        
        vector<int> candidates;
        for (size_t i = 0; i < batch.size(); i++) {
            int r_first = RESP[2*i];
            if (r_first == 0) candidates.push_back(batch[i]);
        }
        
        if (candidates.empty()) continue;
        
        // Commit query: emit candidates
        send_query(candidates);
        
        // Track which became adjacent: response after token c_i is adjacency count of S after toggling c_i
        // Background was 0 (S = A, independent). After toggling c_i in, adjacency = (neighbors of c_i in S).
        // If c_i has no ring neighbor in A nor in previously-added c_j's, response = 0, commit it.
        // If response = 1, c_i became adjacent to a previously committed c_j -> conflict.
        // But response is cumulative; let's track running response.
        // Actually each response is the total adjacency count of current S (which is a set).
        // S starts as A (all independent, count=0). We toggle c_1 in: response = adjacencies of c_1 with rest of S.
        // We need to decide: if response increases, that's a conflict. We toggle out conflicts in cleanup.
        
        vector<int> conflicts;
        vector<char> committed(candidates.size(), 0);
        int prev_resp = 0;
        for (size_t i = 0; i < candidates.size(); i++) {
            int r = RESP[i];
            // The change from prev_resp tells us adjacency contribution of c_i
            // Since c_i was not in S before this toggle, after toggle c_i is in S
            // adjacency contribution = number of neighbors of c_i in S\{c_i}
            // = r - prev_resp (since other vertices' adjacencies unchanged... actually not, adjacencies double count)
            // Total adjacency sum counts each edge twice. So new edges added = (r - prev_resp)/2.
            // If r > prev_resp, c_i has neighbors among committed.
            if (r > prev_resp) {
                conflicts.push_back(candidates[i]);
            } else {
                committed[i] = 1;
            }
            prev_resp = r;
        }
        
        // Update A_list: add committed candidates, but they're still in S. Conflicts need to be removed.
        for (size_t i = 0; i < candidates.size(); i++) {
            if (committed[i]) {
                A_list.push_back(candidates[i]);
                inA[candidates[i]] = 1;
            }
        }
        
        // Cleanup: toggle out conflicts
        if (!conflicts.empty()) {
            send_query(conflicts);
        }
    }
    
    // Now S = A_list (a maximal independent set on the ring)
    int m = (int)A_list.size();
    if (m == 0) {
        // Shouldn't happen for n>1000
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
        return 0;
    }
    
    int K = 1;
    while ((1 << K) < m) K++;
    if (K > 17) K = 17;
    if (K < 1) K = 1;
    
    auto bitrev = [&](int x) {
        int r = 0;
        for (int b = 0; b < K; b++) if (x & (1 << b)) r |= (1 << (K - 1 - b));
        return r;
    };
    
    vector<int> idx1(m), idx2(m);
    for (int i = 0; i < m; i++) { idx1[i] = i; idx2[i] = bitrev(i); }
    
    vector<int> nonA;
    for (int x = 1; x <= n; x++) if (!inA[x]) nonA.push_back(x);
    
    auto do_signature_query = [&](const vector<int>& idx, vector<int>& sig_out) {
        // S currently = A_list. We process K bits.
        // For each bit b, T_b = { A_list[i] : idx[i] has bit b set }
        // Transition: from current T_prev to T_b: toggle symmetric difference.
        // Then probe each nonA y with pair y y.
        // Read responses, decode.
        
        vector<int> tokens;
        vector<char> cur_in(m, 1); // currently A_list[i] in S
        // Track positions where probe responses start, per bit
        vector<int> probe_start(K);
        
        for (int b = 0; b < K; b++) {
            // target: cur_in[i] should be 1 if idx[i] has bit b
            // First emit toggle-out tokens (those currently in but shouldn't be)
            for (int i = 0; i < m; i++) {
                bool want = (idx[i] >> b) & 1;
                if (cur_in[i] && !want) {
                    tokens.push_back(A_list[i]);
                    cur_in[i] = 0;
                }
            }
            // Then emit toggle-in tokens
            for (int i = 0; i < m; i++) {
                bool want = (idx[i] >> b) & 1;
                if (!cur_in[i] && want) {
                    tokens.push_back(A_list[i]);
                    cur_in[i] = 1;
                }
            }
            probe_start[b] = tokens.size();
            for (int y : nonA) { tokens.push_back(y); tokens.push_back(y); }
        }
        // Final: restore S to A. Toggle in those not currently in.
        for (int i = 0; i < m; i++) {
            if (!cur_in[i]) {
                tokens.push_back(A_list[i]);
                cur_in[i] = 1;
            }
        }
        
        send_query(tokens);
        
        sig_out.assign(nonA.size(), 0);
        for (int b = 0; b < K; b++) {
            int ps = probe_start[b];
            for (size_t j = 0; j < nonA.size(); j++) {
                int r_first = RESP[ps + 2*j]; // response after first y token
                if (r_first > 0) sig_out[j] |= (1 << b);
            }
        }
    };
    
    vector<int> sig1, sig2;
    do_signature_query(idx1, sig1);
    do_signature_query(idx2, sig2);
    
    vector<vector<int>> neighbors(n + 1);
    
    // Decode
    vector<int> double_gap_vertex; // y values
    vector<int> double_gap_aidx;   // single A neighbor index
    
    for (size_t j = 0; j < nonA.size(); j++) {
        int y = nonA[j];
        int s1 = sig1[j], s2 = sig2[j];
        
        // Check single A neighbor case
        if (s1 < m && idx2[s1] == s2) {
            int ai = s1;
            double_gap_vertex.push_back(y);
            double_gap_aidx.push_back(ai);
            neighbors[y].push_back(A_list[ai]);
            neighbors[A_list[ai]].push_back(y);
        } else {
            // Find pair i<j with idx1[i]|idx1[j] = s1, idx2[i]|idx2[j] = s2
            // i.e., i|j = s1 (since idx1[k]=k)
            // Enumerate submasks of s1
            int found_i = -1, found_j = -1;
            int sub = s1;
            while (true) {
                int comp = s1 ^ sub;
                if (sub < comp) {
                    int i = sub, jj = comp;
                    if (i < m && jj < m) {
                        if ((idx2[i] | idx2[jj]) == s2) {
                            found_i = i; found_j = jj;
                            break;
                        }
                    }
                }
                if (sub == 0) break;
                sub = (sub - 1) & s1;
            }
            if (found_i >= 0) {
                neighbors[y].push_back(A_list[found_i]);
                neighbors[y].push_back(A_list[found_j]);
                neighbors[A_list[found_i]].push_back(y);
                neighbors[A_list[found_j]].push_back(y);
            }
        }
    }
    
    // Step 4: partner detection among double gap vertices
    // Group double gap vertices by aidx
    vector<vector<int>> groups(m); // groups[a] = list of y values
    for (size_t k = 0; k < double_gap_vertex.size(); k++) {
        groups[double_gap_aidx[k]].push_back(double_gap_vertex[k]);
    }
    
    vector<pair<int,int>> pairs_to_test;
    for (int a = 0; a < m; a++) {
        auto& g = groups[a];
        for (size_t i = 0; i < g.size(); i++) {
            for (size_t j = i + 1; j < g.size(); j++) {
                pairs_to_test.push_back({g[i], g[j]});
            }
        }
    }
    
    if (!pairs_to_test.empty()) {
        vector<int> tokens;
        // First toggle S to empty: send A_list
        for (int a : A_list) tokens.push_back(a);
        int pair_start = tokens.size();
        for (auto& p : pairs_to_test) {
            tokens.push_back(p.first);
            tokens.push_back(p.second);
            tokens.push_back(p.second);
            tokens.push_back(p.first);
        }
        send_query(tokens);
        
        for (size_t k = 0; k < pairs_to_test.size(); k++) {
            int base = pair_start + 4*k;
            int r_second = RESP[base + 1]; // after y' insertion, S = {y, y'}
            if (r_second >= 2) { // edge means adjacency sum = 2
                int y = pairs_to_test[k].first;
                int yp = pairs_to_test[k].second;
                neighbors[y].push_back(yp);
                neighbors[yp].push_back(y);
            }
        }
    }
    
    // Verify
    bool ok = true;
    for (int v = 1; v <= n; v++) {
        if ((int)neighbors[v].size() != 2) { ok = false; break; }
    }
    
    if (!ok) {
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
        return 0;
    }
    
    printf("-1");
    int prev = 0, cur = 1;
    for (int i = 0; i < n; i++) {
        printf(" %d", cur);
        int nxt;
        if (neighbors[cur][0] != prev) nxt = neighbors[cur][0];
        else nxt = neighbors[cur][1];
        prev = cur;
        cur = nxt;
    }
    printf("\n");
    fflush(stdout);
    return 0;
}