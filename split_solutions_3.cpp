#include <cstdio>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

static int readInt() {
    int x;
    scanf("%d", &x);
    return x;
}

int main() {
    int subtask, n;
    scanf("%d %d", &subtask, &n);
    
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
    
    // n > 1000 path
    const int K = 17;
    
    // We will output tokens and read responses interactively-like, but problem format
    // appears to be: output length then all tokens, then read all responses.
    // Since we need adaptive behavior in Phase A, we use the interactive form where 
    // we open queries with length, then exchange tokens/responses.
    // 
    // Approach assumes interactive: print length first... but we need adaptive.
    // We'll build tokens buffer and responses buffer; but Phase A needs response to decide.
    //
    // Re-reading: "Open query one" suggests we print something to open, then alternate.
    // Given n<=1000 used precomputed batch, for n>1000 we need interactive.
    // We'll assume: print a single token, fflush, then read response. Server reads tokens one at a time.
    // But then how is "query" opened? Maybe print "?" or print count first.
    //
    // Given ambiguity, I'll implement assuming we just emit tokens and read responses 
    // one-by-one with fflush, and each "query" is just a logical grouping with no protocol marker.
    // Actually based on n<=1000 code, the protocol prints length L then L tokens then reads L responses.
    // For adaptive we'd need different protocol. I'll assume for n>1000 we print "?" and length, 
    // but spec says "Open query". Let me just print each token then flush then read response.
    
    vector<int> A_list;
    set<int> inS;
    
    auto sendRead = [&](int x) -> int {
        printf("%d ", x);
        fflush(stdout);
        return readInt();
    };
    
    // Phase A
    for (int x = 1; x <= n; x++) {
        int r = sendRead(x);
        if (r == 1) {
            // send x again to toggle out
            sendRead(x);
        } else {
            // x added to S
            inS.insert(x);
            A_list.push_back(x);
        }
    }
    
    int m = A_list.size();
    // position of each label in A_list
    vector<int> posInA(n + 1, -1);
    for (int i = 0; i < m; i++) posInA[A_list[i]] = i;
    
    auto bitReverse = [&](int x) -> int {
        int r = 0;
        for (int b = 0; b < K; b++) if (x & (1 << b)) r |= (1 << (K - 1 - b));
        return r;
    };
    
    vector<int> idx1(m), idx2(m);
    for (int i = 0; i < m; i++) {
        idx1[i] = i;
        idx2[i] = bitReverse(i);
    }
    
    vector<int> sig1(n + 1, 0), sig2(n + 1, 0);
    
    auto runPhaseB = [&](vector<int>& idxArr, vector<int>& sigOut) {
        for (int b = 0; b < K; b++) {
            // Transform S to T_b: positions in A_list whose idxArr has bit b set
            // Toggle out: labels currently in S whose idx bit b is 0
            // Toggle in: labels in A_list not currently in S whose idx bit b is 1
            for (int i = 0; i < m; i++) {
                int lab = A_list[i];
                bool wantIn = (idxArr[i] >> b) & 1;
                bool isIn = inS.count(lab) > 0;
                if (isIn && !wantIn) {
                    sendRead(lab);
                    inS.erase(lab);
                }
            }
            for (int i = 0; i < m; i++) {
                int lab = A_list[i];
                bool wantIn = (idxArr[i] >> b) & 1;
                bool isIn = inS.count(lab) > 0;
                if (!isIn && wantIn) {
                    sendRead(lab);
                    inS.insert(lab);
                }
            }
            // For each y not in A: send y twice
            for (int y = 1; y <= n; y++) {
                if (posInA[y] != -1) continue;
                int r1 = sendRead(y);
                if (r1) sigOut[y] |= (1 << b);
                sendRead(y); // should be 0
            }
        }
    };
    
    // Phase B with idx1
    runPhaseB(idx1, sig1);
    
    // Phase C: reset S to full A_list
    for (int i = 0; i < m; i++) {
        int lab = A_list[i];
        if (!inS.count(lab)) {
            sendRead(lab);
            inS.insert(lab);
        }
    }
    // Also remove anything in S not in A_list (shouldn't be any)
    
    runPhaseB(idx2, sig2);
    
    // Phase D
    vector<vector<int>> neighbors(n + 1);
    vector<pair<int,int>> singleGap; // y -> A-neighbor label
    
    for (int y = 1; y <= n; y++) {
        if (posInA[y] != -1) continue;
        int s1 = sig1[y], s2 = sig2[y];
        // Try single candidate: i = s1, check idx2[i] == s2
        bool foundSingle = false;
        if (s1 >= 0 && s1 < m && idx2[s1] == s2) {
            // double-gap: one A-neighbor at position s1
            neighbors[y].push_back(A_list[s1]);
            neighbors[A_list[s1]].push_back(y);
            singleGap.push_back({y, A_list[s1]});
            foundSingle = true;
        }
        if (!foundSingle) {
            // Enumerate submasks of s1, find pair (i,j) i<j, i|j==s1, idx2[i]|idx2[j]==s2
            int found_i = -1, found_j = -1;
            for (int sub = s1; sub > 0; sub = (sub - 1) & s1) {
                int i = sub;
                int j = s1 ^ sub;
                if (i >= j) continue;
                if (i >= m || j >= m) continue;
                if ((idx2[i] | idx2[j]) == s2) {
                    found_i = i; found_j = j;
                    break;
                }
                if (sub == 0) break;
            }
            if (found_i != -1) {
                neighbors[y].push_back(A_list[found_i]);
                neighbors[y].push_back(A_list[found_j]);
                neighbors[A_list[found_i]].push_back(y);
                neighbors[A_list[found_j]].push_back(y);
            }
        }
    }
    
    // Phase F: double-gap vertices need partner; these are vertices with only 1 neighbor so far
    // singleGap stores them
    vector<int> D;
    for (auto& p : singleGap) {
        if ((int)neighbors[p.first].size() < 2) D.push_back(p.first);
    }
    
    // For each unordered pair in D, send y yp yp y from S empty
    // First, reset S to empty
    vector<int> curS(inS.begin(), inS.end());
    for (int lab : curS) {
        sendRead(lab);
    }
    inS.clear();
    
    int dsz = D.size();
    vector<bool> paired(dsz, false);
    for (int i = 0; i < dsz; i++) {
        if (paired[i]) continue;
        for (int j = i + 1; j < dsz; j++) {
            if (paired[j]) continue;
            int y = D[i], yp = D[j];
            int r0 = sendRead(y);
            int r1 = sendRead(yp);
            int r2 = sendRead(yp);
            int r3 = sendRead(y);
            (void)r0; (void)r2; (void)r3;
            if (r1 == 1) {
                neighbors[y].push_back(yp);
                neighbors[yp].push_back(y);
                paired[i] = true;
                paired[j] = true;
                break;
            }
        }
    }
    
    // Phase G: verify
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
        int nxt = (neighbors[cur][0] != prev) ? neighbors[cur][0] : neighbors[cur][1];
        prev = cur;
        cur = nxt;
    }
    printf("\n");
    fflush(stdout);
    return 0;
}