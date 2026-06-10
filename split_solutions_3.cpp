#include <cstdio>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <cstdint>
using namespace std;

int N;

int sendQuery(const vector<int>& tokens) {
    printf("%d", (int)tokens.size());
    for (int t : tokens) printf(" %d", t);
    printf("\n");
    fflush(stdout);
    int r;
    scanf("%d", &r);
    return r;
}

vector<int> sendQueryMulti(const vector<int>& tokens) {
    long long L = (long long)tokens.size();
    printf("%lld", L);
    for (int t : tokens) printf(" %d", t);
    printf("\n");
    fflush(stdout);
    vector<int> res(tokens.size());
    for (size_t i = 0; i < tokens.size(); i++) {
        scanf("%d", &res[i]);
    }
    return res;
}

int main() {
    int subtask, n;
    scanf("%d %d", &subtask, &n);
    N = n;
    
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
            if (v > n) {
                u++;
                v = u + 1;
            }
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
    
    vector<int> A_list;
    vector<int> inA(n + 1, 0);
    
    const int E = 12;
    const uint64_t SEED_BASE = 0xD6E8FEB86659FD93ULL;
    
    for (int e = 0; e < E; e++) {
        vector<int> R;
        for (int v = 1; v <= n; v++) if (!inA[v]) R.push_back(v);
        if (R.empty()) break;
        
        uint64_t x = SEED_BASE ^ (uint64_t)e;
        if (x == 0) x = 1;
        for (int i = (int)R.size() - 1; i >= 1; i--) {
            x ^= x << 13;
            x ^= x >> 7;
            x ^= x << 17;
            uint64_t j = x % (uint64_t)(i + 1);
            swap(R[i], R[(int)j]);
        }
        
        long long L = (long long)R.size();
        printf("%lld", L);
        for (int t : R) printf(" %d", t);
        printf("\n");
        fflush(stdout);
        vector<int> res(R.size());
        for (size_t i = 0; i < R.size(); i++) scanf("%d", &res[i]);
        
        int k = (int)R.size();
        for (int i = 0; i < (int)R.size(); i++) {
            if (res[i] == 1) { k = i; break; }
        }
        
        for (int i = 0; i < k; i++) {
            A_list.push_back(R[i]);
            inA[R[i]] = 1;
        }
        
        if (k < (int)R.size()) {
            vector<int> cleanup;
            for (int i = k; i < (int)R.size(); i++) cleanup.push_back(R[i]);
            long long CL = (long long)cleanup.size();
            printf("%lld", CL);
            for (int t : cleanup) printf(" %d", t);
            printf("\n");
            fflush(stdout);
            for (size_t i = 0; i < cleanup.size(); i++) {
                int tmp; scanf("%d", &tmp);
            }
        }
    }
    
    if ((int)A_list.size() < n / 3) {
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
        return 0;
    }
    
    {
        vector<int> R_rem;
        for (int v = 1; v <= n; v++) if (!inA[v]) R_rem.push_back(v);
        if (!R_rem.empty()) {
            long long L = 2LL * (long long)R_rem.size();
            printf("%lld", L);
            for (int v : R_rem) printf(" %d %d", v, v);
            printf("\n");
            fflush(stdout);
            vector<int> resp(L);
            for (long long i = 0; i < L; i++) scanf("%d", &resp[i]);
            
            vector<int> M;
            for (size_t k = 0; k < R_rem.size(); k++) {
                if (resp[2 * k] == 0) M.push_back(R_rem[k]);
            }
            
            const int E2 = 8;
            const uint64_t SEED2 = 0xCAFEBABEDEADBEEFULL;
            
            for (int e2 = 0; e2 < E2; e2++) {
                vector<int> R2;
                for (int mm : M) if (!inA[mm]) R2.push_back(mm);
                if (R2.empty()) break;
                
                uint64_t x = SEED2 ^ (uint64_t)e2;
                if (x == 0) x = 1;
                for (int i = (int)R2.size() - 1; i >= 1; i--) {
                    x ^= x << 13;
                    x ^= x >> 7;
                    x ^= x << 17;
                    uint64_t j = x % (uint64_t)(i + 1);
                    swap(R2[i], R2[(int)j]);
                }
                
                long long LL = (long long)R2.size();
                printf("%lld", LL);
                for (int t : R2) printf(" %d", t);
                printf("\n");
                fflush(stdout);
                vector<int> res(R2.size());
                for (size_t i = 0; i < R2.size(); i++) scanf("%d", &res[i]);
                
                int k2 = (int)R2.size();
                for (int i = 0; i < (int)R2.size(); i++) {
                    if (res[i] == 1) { k2 = i; break; }
                }
                
                for (int i = 0; i < k2; i++) {
                    A_list.push_back(R2[i]);
                    inA[R2[i]] = 1;
                }
                
                if (k2 < (int)R2.size()) {
                    vector<int> cleanup;
                    for (int i = k2; i < (int)R2.size(); i++) cleanup.push_back(R2[i]);
                    long long CL = (long long)cleanup.size();
                    printf("%lld", CL);
                    for (int t : cleanup) printf(" %d", t);
                    printf("\n");
                    fflush(stdout);
                    for (size_t i = 0; i < cleanup.size(); i++) {
                        int tmp; scanf("%d", &tmp);
                    }
                }
            }
        }
    }
    
    int m = (int)A_list.size();
    int K = 1;
    while ((1 << K) < m) K++;
    if (K > 17) K = 17;
    if (K < 1) K = 1;
    
    vector<int> idx1(m), idx2(m), idx3(m);
    for (int i = 0; i < m; i++) {
        idx1[i] = i;
        idx2[i] = i;
        idx3[i] = i;
    }
    
    {
        uint64_t x = 0x9E3779B97F4A7C15ULL;
        for (int i = m - 1; i >= 1; i--) {
            x ^= x << 13;
            x ^= x >> 7;
            x ^= x << 17;
            uint64_t j = x % (uint64_t)(i + 1);
            swap(idx2[i], idx2[(int)j]);
        }
    }
    {
        uint64_t x = 0xBF58476D1CE4E5B9ULL;
        for (int i = m - 1; i >= 1; i--) {
            x ^= x << 13;
            x ^= x >> 7;
            x ^= x << 17;
            uint64_t j = x % (uint64_t)(i + 1);
            swap(idx3[i], idx3[(int)j]);
        }
    }
    
    vector<int> nonA;
    for (int v = 1; v <= n; v++) if (!inA[v]) nonA.push_back(v);
    int nn = (int)nonA.size();
    int numNonA = nn;
    
    vector<int> sig1(n + 1, 0), sig2(n + 1, 0), sig3(n + 1, 0);
    
    for (int round = 0; round < 3; round++) {
        vector<int>& L = (round == 0) ? idx1 : (round == 1 ? idx2 : idx3);
        vector<int> tokens;
        vector<int> cur_in(m, 1);
        vector<int> probe_start(K);
        
        for (int b = 0; b < K; b++) {
            for (int i = 0; i < m; i++) {
                int want = (L[i] >> b) & 1;
                if (cur_in[i] == 1 && want == 0) {
                    tokens.push_back(A_list[i]);
                    cur_in[i] = 0;
                }
            }
            for (int i = 0; i < m; i++) {
                int want = (L[i] >> b) & 1;
                if (cur_in[i] == 0 && want == 1) {
                    tokens.push_back(A_list[i]);
                    cur_in[i] = 1;
                }
            }
            probe_start[b] = (int)tokens.size();
            for (int y : nonA) {
                tokens.push_back(y);
                tokens.push_back(y);
            }
        }
        for (int i = 0; i < m; i++) {
            if (cur_in[i] == 0) {
                tokens.push_back(A_list[i]);
                cur_in[i] = 1;
            }
        }
        
        vector<int> resp = sendQueryMulti(tokens);
        
        for (int b = 0; b < K; b++) {
            int base = probe_start[b];
            for (int j = 0; j < nn; j++) {
                int r = resp[base + 2 * j];
                if (r) {
                    if (round == 0) sig1[nonA[j]] |= (1 << b);
                    else if (round == 1) sig2[nonA[j]] |= (1 << b);
                    else sig3[nonA[j]] |= (1 << b);
                }
            }
        }
    }
    
    vector<int> sig1Of(numNonA), sig2Of(numNonA), sig3Of(numNonA);
    for (int q = 0; q < numNonA; q++) {
        sig1Of[q] = sig1[nonA[q]];
        sig2Of[q] = sig2[nonA[q]];
        sig3Of[q] = sig3[nonA[q]];
    }
    
    vector<vector<int>> neighbors(n + 1);
    set<pair<int,int>> edgeSet;
    set<pair<int,int>> skelEdges;
    auto pushNeighbor = [&](int a, int b) {
        int aa = a, bb = b;
        if (aa > bb) swap(aa, bb);
        if (edgeSet.count({aa,bb})) return;
        edgeSet.insert({aa,bb});
        neighbors[a].push_back(b);
        neighbors[b].push_back(a);
    };
    
    vector<int> yType(n + 1, 0);
    vector<int> ySoleA(n + 1, -1);
    
    vector<vector<int>> bySoleA(m);
    vector<int> D;
    vector<int> degA(m, 0);
    
    auto fallback = [&]() {
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
    };
    
    // Phase 3: bucket by sig1
    vector<vector<int>> bucketBy_S1(1 << K);
    for (int q = 0; q < numNonA; q++) {
        bucketBy_S1[sig1Of[q]].push_back(q);
    }
    
    vector<int> dgCount(numNonA, 0);
    vector<int> dgSub(numNonA, -1);
    vector<int> sgCount(numNonA, 0);
    vector<pair<int,int>> sgPair(numNonA, {-1, -1});
    
    // Step 3a: single A index
    for (int i = 0; i < m; i++) {
        int s1_i = idx1[i];
        int s2_i = idx2[i];
        int s3_i = idx3[i];
        for (int q : bucketBy_S1[s1_i]) {
            if (sig2Of[q] == s2_i && sig3Of[q] == s3_i) {
                dgCount[q]++;
                dgSub[q] = i;
            }
        }
    }
    
    // Step 3b: unordered pair
    for (int i = 0; i < m; i++) {
        int idx2_i = idx2[i];
        int idx3_i = idx3[i];
        for (int j = i + 1; j < m; j++) {
            int s1_ij = i | j;
            int s2_ij = idx2_i | idx2[j];
            int s3_ij = idx3_i | idx3[j];
            for (int q : bucketBy_S1[s1_ij]) {
                if (sig2Of[q] == s2_ij && sig3Of[q] == s3_ij) {
                    sgCount[q]++;
                    sgPair[q] = {i, j};
                }
            }
        }
    }
    
    // Step 3c: classify
    for (int q = 0; q < numNonA; q++) {
        int y = nonA[q];
        if (dgCount[q] == 1 && sgCount[q] == 0) {
            int i = dgSub[q];
            pushNeighbor(y, A_list[i]);
            D.push_back(y);
            bySoleA[i].push_back(y);
            yType[y] = 1;
            ySoleA[y] = i;
        } else if (dgCount[q] == 0 && sgCount[q] == 1) {
            int i = sgPair[q].first;
            int j = sgPair[q].second;
            pushNeighbor(y, A_list[i]);
            pushNeighbor(y, A_list[j]);
            int aa = i, bb = j;
            if (aa > bb) swap(aa, bb);
            skelEdges.insert({aa, bb});
            degA[i]++;
            degA[j]++;
            yType[y] = 2;
        }
    }
    
    for (int i = 0; i < m; i++) {
        if (degA[i] > 2) { fallback(); return 0; }
    }
    
    {
        vector<int> resp = sendQueryMulti(A_list);
        (void)resp;
    }
    
    vector<int> T;
    vector<int> inT(n + 1, 0);
    {
        const int E_T = 16;
        const uint64_t SEED_T = 0xA0761D6478BD642FULL;
        for (int e = 0; e < E_T; e++) {
            vector<int> R;
            for (int y : D) if (!inT[y]) R.push_back(y);
            if (R.empty()) break;
            
            uint64_t x = SEED_T ^ (uint64_t)e;
            if (x == 0) x = 1;
            for (int i = (int)R.size() - 1; i >= 1; i--) {
                x ^= x << 13;
                x ^= x >> 7;
                x ^= x << 17;
                uint64_t j = x % (uint64_t)(i + 1);
                swap(R[i], R[(int)j]);
            }
            
            long long L = (long long)R.size();
            printf("%lld", L);
            for (int t : R) printf(" %d", t);
            printf("\n");
            fflush(stdout);
            vector<int> res(R.size());
            for (size_t i = 0; i < R.size(); i++) scanf("%d", &res[i]);
            
            int k = (int)R.size();
            for (int i = 0; i < (int)R.size(); i++) {
                if (res[i] == 1) { k = i; break; }
            }
            
            for (int i = 0; i < k; i++) {
                T.push_back(R[i]);
                inT[R[i]] = 1;
            }
            
            if (k < (int)R.size()) {
                vector<int> cleanup;
                for (int i = k; i < (int)R.size(); i++) cleanup.push_back(R[i]);
                long long CL = (long long)cleanup.size();
                printf("%lld", CL);
                for (int t : cleanup) printf(" %d", t);
                printf("\n");
                fflush(stdout);
                for (size_t i = 0; i < cleanup.size(); i++) {
                    int tmp; scanf("%d", &tmp);
                }
            }
        }
    }
    
    {
        vector<int> U;
        for (int y : D) if (!inT[y]) U.push_back(y);
        if (!U.empty()) {
            vector<int> tokens;
            tokens.reserve(2 * U.size());
            for (int y : U) { tokens.push_back(y); tokens.push_back(y); }
            vector<int> resp = sendQueryMulti(tokens);
            
            vector<int> MM;
            for (size_t p = 0; p < U.size(); p++) {
                if (resp[2 * p] == 0) MM.push_back(U[p]);
            }
            
            const int E_TT = 8;
            const uint64_t SEED_TT = 0x4CF5AD432745937FULL;
            for (int e = 0; e < E_TT; e++) {
                vector<int> R2;
                for (int y : MM) if (!inT[y]) R2.push_back(y);
                if (R2.empty()) break;
                
                uint64_t x = SEED_TT ^ (uint64_t)e;
                if (x == 0) x = 1;
                for (int i = (int)R2.size() - 1; i >= 1; i--) {
                    x ^= x << 13;
                    x ^= x >> 7;
                    x ^= x << 17;
                    uint64_t j = x % (uint64_t)(i + 1);
                    swap(R2[i], R2[(int)j]);
                }
                
                long long LL = (long long)R2.size();
                printf("%lld", LL);
                for (int t : R2) printf(" %d", t);
                printf("\n");
                fflush(stdout);
                vector<int> res(R2.size());
                for (size_t i = 0; i < R2.size(); i++) scanf("%d", &res[i]);
                
                int k2 = (int)R2.size();
                for (int i = 0; i < (int)R2.size(); i++) {
                    if (res[i] == 1) { k2 = i; break; }
                }
                
                for (int i = 0; i < k2; i++) {
                    T.push_back(R2[i]);
                    inT[R2[i]] = 1;
                }
                
                if (k2 < (int)R2.size()) {
                    vector<int> cleanup;
                    for (int i = k2; i < (int)R2.size(); i++) cleanup.push_back(R2[i]);
                    long long CL = (long long)cleanup.size();
                    printf("%lld", CL);
                    for (int t : cleanup) printf(" %d", t);
                    printf("\n");
                    fflush(stdout);
                    for (size_t i = 0; i < cleanup.size(); i++) {
                        int tmp; scanf("%d", &tmp);
                    }
                }
            }
        }
    }
    
    int mT = (int)T.size();
    if (mT > 0) {
        int K_T = 1;
        while ((1 << K_T) < mT) K_T++;
        if (K_T > 17) K_T = 17;
        if (K_T < 1) K_T = 1;
        
        vector<int> U;
        for (int y : D) if (!inT[y]) U.push_back(y);
        int dU = (int)U.size();
        
        vector<int> tokens;
        vector<int> cur_in(mT, 1);
        vector<int> probe_start(K_T);
        
        for (int b = 0; b < K_T; b++) {
            for (int j = 0; j < mT; j++) {
                int want = (j >> b) & 1;
                if (cur_in[j] == 1 && want == 0) {
                    tokens.push_back(T[j]);
                    cur_in[j] = 0;
                }
            }
            for (int j = 0; j < mT; j++) {
                int want = (j >> b) & 1;
                if (cur_in[j] == 0 && want == 1) {
                    tokens.push_back(T[j]);
                    cur_in[j] = 1;
                }
            }
            probe_start[b] = (int)tokens.size();
            for (int y : U) {
                tokens.push_back(y);
                tokens.push_back(y);
            }
        }
        for (int j = 0; j < mT; j++) {
            if (cur_in[j] == 0) {
                tokens.push_back(T[j]);
                cur_in[j] = 1;
            }
        }
        
        vector<int> resp = sendQueryMulti(tokens);
        
        vector<int> partner_label(dU, 0);
        for (int b = 0; b < K_T; b++) {
            int base = probe_start[b];
            for (int q = 0; q < dU; q++) {
                int r = resp[base + 2 * q];
                if (r) partner_label[q] |= (1 << b);
            }
        }
        
        for (int q = 0; q < dU; q++) {
            int lbl = partner_label[q];
            if (lbl >= mT) { fallback(); return 0; }
            pushNeighbor(U[q], T[lbl]);
        }
    }
    
    bool ok = true;
    for (int v = 1; v <= n; v++) {
        if ((int)neighbors[v].size() != 2) { ok = false; break; }
    }
    
    if (!ok) { fallback(); return 0; }
    
    vector<int> walk(n);
    vector<int> visited(n + 1, 0);
    int prev = 0, cur = 1;
    for (int i = 0; i < n; i++) {
        walk[i] = cur;
        if (cur < 1 || cur > n || visited[cur]) { ok = false; break; }
        visited[cur] = 1;
        int nxt;
        if (neighbors[cur][0] != prev) nxt = neighbors[cur][0];
        else nxt = neighbors[cur][1];
        prev = cur;
        cur = nxt;
    }
    if (!ok) { fallback(); return 0; }
    for (int i = 1; i <= n; i++) if (!visited[i]) { ok = false; break; }
    if (!ok) { fallback(); return 0; }
    
    printf("-1");
    for (int i = 0; i < n; i++) printf(" %d", walk[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}