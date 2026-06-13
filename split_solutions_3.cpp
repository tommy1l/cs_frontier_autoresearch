#include <bits/stdc++.h>
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int subtask, n;
    cin >> subtask >> n;

    if (n <= 1000) {
        long long L = 2LL * n * (n - 1);
        cout << L;
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                cout << ' ' << i << ' ' << j << ' ' << j << ' ' << i;
            }
        }
        cout << '\n';
        cout.flush();

        vector<int> resp(L);
        for (long long k = 0; k < L; k++) cin >> resp[k];

        vector<vector<int>> adj(n + 1);
        long long idx = 0;
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                if (resp[idx + 1] == 1) {
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                }
                idx += 4;
            }
        }

        vector<int> perm;
        perm.reserve(n);
        perm.push_back(1);
        int prev = -1, cur = 1;
        for (int step = 1; step < n; step++) {
            int nxt = -1;
            for (int v : adj[cur]) if (v != prev) { nxt = v; break; }
            if (nxt == -1) break;
            perm.push_back(nxt);
            prev = cur;
            cur = nxt;
        }

        cout << -1;
        for (int x : perm) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    } else {
        // NEW REGIME "global-fp":
        // TRY-#1 from DIRECTIVES: bit-encoding at GLOBAL granularity, every vertex
        // gets a fingerprint. BITS = ceil(log2 n) = 17. For each bit b, S_b is a
        // no-adj-likely subset of {v : bit b of v on}, capped at ~sqrt(n)=316 so
        // that expected adj pairs in S_b is ~1 (sub-Poisson).
        //
        // Each non-S_b vertex v is probed with (v, v); the response after the first
        // toggle tells whether v has a ring-neighbor in S_b. Across 17 bits, fp[v]
        // is the bitwise OR over {bit b : (some nbr of v) in S_b}.
        //
        // This trial only collects fp[]; it does NOT attempt decoding into edges.
        // Output is identity. This anchor measures (a) regime cost: 17 queries,
        // ~3.4e6 ops, (b) whether global-granularity probing under the
        // |S_b| <= sqrt(n) cap is even feasible at n=1e5.

        const int BITS = 17;
        const int Sb_CAP = 316;

        vector<int> fp(n + 1, 0);

        for (int b = 0; b < BITS; b++) {
            vector<int> Sb;
            for (int v = 1; v <= n; v++) {
                if (((v >> b) & 1) && (int)Sb.size() < Sb_CAP) Sb.push_back(v);
            }

            vector<char> inSb(n + 1, 0);
            for (int x : Sb) inSb[x] = 1;

            vector<int> ops;
            ops.reserve(Sb.size() * 2 + 2 * (n - Sb.size()));
            for (int x : Sb) ops.push_back(x);
            for (int v = 1; v <= n; v++) {
                if (!inSb[v]) {
                    ops.push_back(v);
                    ops.push_back(v);
                }
            }
            for (int x : Sb) ops.push_back(x);

            long long L = ops.size();
            cout << L;
            for (int x : ops) cout << ' ' << x;
            cout << '\n';
            cout.flush();

            vector<int> resp(L);
            for (long long k = 0; k < L; k++) cin >> resp[k];

            long long idx = (long long)Sb.size();
            for (int v = 1; v <= n; v++) {
                if (!inSb[v]) {
                    if (resp[idx] == 1) fp[v] |= (1 << b);
                    idx += 2;
                }
            }
        }

        cout << -1;
        for (int v = 1; v <= n; v++) cout << ' ' << v;
        cout << '\n';
        cout.flush();
    }
    return 0;
}
