#include <cstdio>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>
using namespace std;

int N;

int sendQuery(const vector<int>& tokens) {
    // returns the single response; tokens.size() should be small here
    printf("%d", (int)tokens.size());
    for (int t : tokens) printf(" %d", t);
    printf("\n");
    fflush(stdout);
    int r;
    scanf("%d", &r);
    return r;
}

vector<int> sendQueryMulti(const vector<int>& tokens) {
    printf("%d", (int)tokens.size());
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
    // Phase 1
    vector<int> A_list;
    vector<int> inA(n + 1, 0);
    for (int v = 1; v <= n; v++) {
        vector<int> tk = {v};
        int r = sendQuery(tk);
        if (r == 0) {
            A_list.push_back(v);
            inA[v] = 1;
        } else {
            // toggle back out
            sendQuery(tk);
        }
    }
    
    int m = (int)A_list.size();
    int K = 1;
    while ((1 << K) < m) K++;
    if (K > 17) K = 17;
    if (K < 1) K = 1;
    
    auto bitReverse = [&](int x, int bits) {
        int r = 0;
        for (int i = 0; i < bits; i++) {
            if (x & (1 << i)) r |= (1 << (bits - 1 - i));
        }
        return r;
    };
    
    vector<int> idx1(m), idx2(m);
    for (int i = 0; i < m; i++) {
        idx1[i] = i;
        idx2[i] = bitReverse(i, K);
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
            // transitions to T_b: want cur_in[i] == ((L[i]>>b)&1)
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
        // restore: put back any cur_in == 0
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
    map<pair<int,int>, bool> edgeSet; // unordered edges
    auto addEdge = [&](int a, int b) {
        if (a > b) swap(a, b);
        edgeSet[{a,b}] = true;
    };
    
    vector<vector<int>> neighbors(n + 1);
    auto pushNeighbor = [&](int a, int b) {
        if (a > b) swap(a, b);
        if (edgeSet.count({a,b})) return;
        edgeSet[{a,b}] = true;
        neighbors[a].push_back(b);
        neighbors[b].push_back(a);
    };
    
    vector<int> yType(n + 1, 0); // 0 unresolved, 1 double-gap, 2 single-gap
    vector<int> ySoleA(n + 1, -1); // for double-gap
    vector<pair<int,int>> ySinglePair(n + 1, {-1,-1});
    
    set<pair<int,int>> singleGapAEdges; // pairs of A indices (unordered)
    vector<int> degA(m, 0);
    
    for (int y : nonA) {
        int s1 = sig1[y], s2 = sig2[y];
        if (s1 < m && idx2[s1] == s2) {
            yType[y] = 1;
            ySoleA[y] = s1;
            pushNeighbor(y, A_list[s1]);
            continue;
        }
        // iterate submasks
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
            ySinglePair[y] = {foundSub, foundComp};
            pushNeighbor(y, A_list[foundSub]);
            pushNeighbor(y, A_list[foundComp]);
            auto pr = make_pair(foundSub, foundComp);
            if (!singleGapAEdges.count(pr)) {
                singleGapAEdges.insert(pr);
                degA[foundSub]++;
                degA[foundComp]++;
            }
        }
    }
    
    // Phase 4: partner detection for double-gap
    vector<int> need(m);
    for (int i = 0; i < m; i++) need[i] = 2 - degA[i];
    
    // group double-gap y by sole A index
    vector<vector<int>> bySoleA(m);
    vector<int> D;
    for (int y : nonA) {
        if (yType[y] == 1) {
            D.push_back(y);
            bySoleA[ySoleA[y]].push_back(y);
        }
    }
    
    // build candidate pairs
    vector<pair<int,int>> candidates;
    int dsz = (int)D.size();
    // For efficiency, iterate pairs but skip if same a_y; also need[a_y] > 0
    for (int i = 0; i < dsz; i++) {
        int y = D[i];
        int ay = ySoleA[y];
        if (need[ay] <= 0) continue;
        for (int j = i + 1; j < dsz; j++) {
            int yp = D[j];
            int ayp = ySoleA[yp];
            if (ay == ayp) continue;
            if (need[ayp] <= 0) continue;
            int a = ay, b = ayp;
            if (a > b) swap(a, b);
            if (singleGapAEdges.count({a, b})) continue;
            candidates.push_back({y, yp});
        }
    }
    
    if (!candidates.empty()) {
        vector<int> tokens;
        // first toggle out all A
        for (int v : A_list) tokens.push_back(v);
        for (auto& pr : candidates) {
            int y = pr.first, yp = pr.second;
            tokens.push_back(y);
            tokens.push_back(yp);
            tokens.push_back(yp);
            tokens.push_back(y);
        }
        vector<int> resp = sendQueryMulti(tokens);
        // The first m responses are during A toggles - ignore
        int base = m;
        for (size_t k = 0; k < candidates.size(); k++) {
            int idx = base + 4 * (int)k;
            int r = resp[idx + 1]; // second position within block
            if (r == 1) {
                int y = candidates[k].first;
                int yp = candidates[k].second;
                pushNeighbor(y, yp);
            }
            // Note: state goes back to empty after each block (y yp yp y toggles add y, add yp, remove yp, remove y)
        }
    }
    
    // Phase 5: verify
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