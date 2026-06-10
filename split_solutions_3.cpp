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
    
    // n > 1000
    // Phase 1: build MIS A
    vector<int> A_list;
    vector<int> inA(n + 1, 0);
    for (int v = 1; v <= n; v++) {
        vector<int> tk = {v};
        int r = sendQuery(tk);
        if (r == 0) {
            A_list.push_back(v);
            inA[v] = 1;
        } else {
            sendQuery(tk);
        }
    }
    
    int m = (int)A_list.size();
    int K = 1;
    while ((1 << K) < m) K++;
    if (K > 17) K = 17;
    if (K < 1) K = 1;
    
    vector<int> idx1(m), idx2(m);
    for (int i = 0; i < m; i++) {
        idx1[i] = i;
        idx2[i] = i;
    }
    
    // Fisher-Yates shuffle idx2 with xorshift64
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
    
    vector<int> nonA;
    for (int v = 1; v <= n; v++) if (!inA[v]) nonA.push_back(v);
    int nn = (int)nonA.size();
    
    vector<int> sig1(n + 1, 0), sig2(n + 1, 0);
    
    for (int round = 0; round < 2; round++) {
        vector<int>& L = (round == 0) ? idx1 : idx2;
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
                    else sig2[nonA[j]] |= (1 << b);
                }
            }
        }
    }
    
    // Phase 3 decode
    vector<vector<int>> neighbors(n + 1);
    set<pair<int,int>> edgeSet;
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
    
    set<pair<int,int>> skelEdges;
    vector<int> degA(m, 0);
    vector<vector<int>> bySoleA(m);
    vector<int> D;
    
    auto popcnt = [](int x){ return __builtin_popcount(x); };
    
    for (int y : nonA) {
        int s1 = sig1[y], s2 = sig2[y];
        if (popcnt(s1) > 14) continue;
        if (s1 < m && idx2[s1] == s2) {
            yType[y] = 1;
            ySoleA[y] = s1;
            pushNeighbor(y, A_list[s1]);
            D.push_back(y);
            bySoleA[s1].push_back(y);
            continue;
        }
        int count = 0;
        int foundSub = -1, foundComp = -1;
        int sub = s1;
        while (true) {
            int comp = s1 ^ sub;
            if (sub < comp && sub < m && comp < m) {
                if ((idx2[sub] | idx2[comp]) == s2) {
                    count++;
                    foundSub = sub;
                    foundComp = comp;
                    if (count > 1) break;
                }
            }
            if (sub == 0) break;
            sub = (sub - 1) & s1;
        }
        if (count == 1) {
            yType[y] = 2;
            pushNeighbor(y, A_list[foundSub]);
            pushNeighbor(y, A_list[foundComp]);
            auto pr = make_pair(foundSub, foundComp);
            if (!skelEdges.count(pr)) {
                skelEdges.insert(pr);
                degA[foundSub]++;
                degA[foundComp]++;
            }
        }
    }
    
    auto fallback = [&]() {
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
    };
    
    // Phase 4: skeleton
    bool ok = true;
    for (int i = 0; i < m; i++) if (degA[i] > 2) { ok = false; break; }
    if (!ok) { fallback(); return 0; }
    
    vector<vector<int>> adjA(m);
    for (auto& e : skelEdges) {
        adjA[e.first].push_back(e.second);
        adjA[e.second].push_back(e.first);
    }
    
    vector<int> compId(m, -1);
    vector<vector<int>> compEndpoints; // endpoints per component
    int cc = 0;
    for (int i = 0; i < m; i++) {
        if (compId[i] != -1) continue;
        // BFS
        vector<int> comp;
        vector<int> stk = {i};
        compId[i] = cc;
        while (!stk.empty()) {
            int u = stk.back(); stk.pop_back();
            comp.push_back(u);
            for (int v : adjA[u]) if (compId[v] == -1) {
                compId[v] = cc;
                stk.push_back(v);
            }
        }
        vector<int> eps;
        for (int u : comp) if ((int)adjA[u].size() <= 1) eps.push_back(u);
        compEndpoints.push_back(eps);
        cc++;
    }
    
    // Phase 5: partner detection
    for (int i = 0; i < m; i++) {
        if ((int)bySoleA[i].size() + degA[i] != 2) { ok = false; break; }
    }
    if (!ok) { fallback(); return 0; }
    
    // candidates across components, only endpoints
    vector<pair<int,int>> candidates;
    long long candCount = 0;
    
    // for each component, list endpoints with c>0
    vector<vector<int>> compEndsWithC(cc);
    for (int c = 0; c < cc; c++) {
        for (int u : compEndpoints[c]) {
            if (!bySoleA[u].empty()) compEndsWithC[c].push_back(u);
        }
    }
    
    for (int c1 = 0; c1 < cc; c1++) {
        for (int c2 = c1 + 1; c2 < cc; c2++) {
            for (int i : compEndsWithC[c1]) {
                for (int j : compEndsWithC[c2]) {
                    for (int y : bySoleA[i]) {
                        for (int yp : bySoleA[j]) {
                            candidates.push_back({y, yp});
                            candCount++;
                            if (candCount > 1500000) { fallback(); return 0; }
                        }
                    }
                }
            }
        }
    }
    
    long long totalTokens = (long long)m + 4LL * candCount;
    if (totalTokens > 9500000LL) { fallback(); return 0; }
    
    if (!candidates.empty() || m > 0) {
        vector<int> tokens;
        tokens.reserve((size_t)totalTokens);
        for (int v : A_list) tokens.push_back(v);
        for (auto& pr : candidates) {
            int y = pr.first, yp = pr.second;
            tokens.push_back(y);
            tokens.push_back(yp);
            tokens.push_back(yp);
            tokens.push_back(y);
        }
        vector<int> resp = sendQueryMulti(tokens);
        int base = m;
        for (size_t k = 0; k < candidates.size(); k++) {
            int idx = base + 4 * (int)k;
            int r = resp[idx + 1];
            if (r == 1) {
                int y = candidates[k].first;
                int yp = candidates[k].second;
                pushNeighbor(y, yp);
            }
        }
    }
    
    // Phase 6: verify
    for (int v = 1; v <= n; v++) {
        if ((int)neighbors[v].size() != 2) { ok = false; break; }
    }
    
    if (!ok) { fallback(); return 0; }
    
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