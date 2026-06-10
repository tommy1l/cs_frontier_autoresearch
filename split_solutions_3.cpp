#include <cstdio>
#include <vector>
using namespace std;

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
        int idx = 0;
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
    } else {
        printf("-1");
        for (int i = 1; i <= n; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
    }
    return 0;
}