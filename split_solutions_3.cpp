#include <bits/stdc++.h>
using namespace std;

int N;
long long opsUsed = 0;
vector<char> inSet;

vector<int> doQuery(const vector<int>& ops) {
    long long L = ops.size();
    cout << L;
    for (int x : ops) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    opsUsed += L;
    vector<int> resp(L);
    for (long long k = 0; k < L; k++) cin >> resp[k];
    for (int x : ops) inSet[x] ^= 1;
    return resp;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int subtask;
    cin >> subtask >> N;
    inSet.assign(N + 1, 0);

    if (N <= 1000) {
        long long L = 2LL * N * (N - 1);
        cout << L;
        for (int i = 1; i <= N; i++) {
            for (int j = i + 1; j <= N; j++) {
                cout << ' ' << i << ' ' << j << ' ' << j << ' ' << i;
            }
        }
        cout << '\n';
        cout.flush();

        vector<int> resp(L);
        for (long long k = 0; k < L; k++) cin >> resp[k];

        vector<vector<int>> adj(N + 1);
        long long idx = 0;
        for (int i = 1; i <= N; i++) {
            for (int j = i + 1; j <= N; j++) {
                if (resp[idx + 1] == 1) {
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                }
                idx += 4;
            }
        }

        vector<int> perm;
        perm.reserve(N);
        perm.push_back(1);
        int prev = -1, cur = 1;
        for (int step = 1; step < N; step++) {
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
        return 0;
    }

    // n large: bit-fingerprint Phase 0 + ITERATIVE Phase 1 endpoint expansion.
    // No in-walk per-step (v,v) probe (DIRECTIVES.AVOID).
    const int K = 49;
    const int BITS_K = 6;
    const long long opCap = 14000000LL;

    // Phase 0: identify anchor neighbors via log2(K) bit queries
    vector<int> sigVals(N + 1, 0);
    for (int b = 0; b < BITS_K; b++) {
        vector<int> ops;
        int transCount = 0;
        for (int a = 1; a <= K; a++) {
            int want = (a >> b) & 1;
            if (want != inSet[a]) {
                ops.push_back(a);
                transCount++;
            }
        }
        for (int v = K + 1; v <= N; v++) {
            ops.push_back(v);
            ops.push_back(v);
        }
        vector<int> resp = doQuery(ops);
        for (int v = K + 1; v <= N; v++) {
            long long idx = (long long)transCount + 2LL * (v - K - 1);
            if (resp[idx] == 1) sigVals[v] |= (1 << b);
        }
    }

    // Cleanup S to empty
    {
        vector<int> ops;
        for (int a = 1; a <= K; a++) if (inSet[a]) ops.push_back(a);
        if (!ops.empty()) doQuery(ops);
    }

    // Initialize chain segments from sigVals
    vector<vector<int>> chains(K + 1);
    vector<char> placed(N + 1, 0);
    {
        vector<vector<int>> anchorNbrs(K + 1);
        for (int v = K + 1; v <= N; v++) {
            int s = sigVals[v];
            if (s >= 1 && s <= K) anchorNbrs[s].push_back(v);
        }
        for (int a = 1; a <= K; a++) {
            chains[a].push_back(a);
            placed[a] = 1;
            if ((int)anchorNbrs[a].size() == 2) {
                int u = anchorNbrs[a][0], w = anchorNbrs[a][1];
                chains[a] = {u, a, w};
                placed[u] = placed[w] = 1;
            } else if ((int)anchorNbrs[a].size() == 1) {
                int u = anchorNbrs[a][0];
                chains[a].push_back(u);
                placed[u] = 1;
            }
        }
    }

    // Iterative Phase 1+: expand chain endpoints via bit-fingerprint.
    // Each iteration uses current chain endpoints (non-anchor, outermost) as
    // fingerprint references, finds each unknown's adjacent endpoint, and
    // extends the chain by 1 on each side. Loops until budget exhausted or
    // no further extension is possible.
    while (true) {
        vector<int> endpoints;
        vector<int> endpointChain;
        vector<int> endpointSide; // 0 = front, 1 = back
        for (int a = 1; a <= K; a++) {
            int sz = chains[a].size();
            if (sz < 2) continue;
            int eFront = chains[a].front();
            int eBack = chains[a].back();
            if (eFront != a) {
                endpoints.push_back(eFront);
                endpointChain.push_back(a);
                endpointSide.push_back(0);
            }
            if (eBack != a && eBack != eFront) {
                endpoints.push_back(eBack);
                endpointChain.push_back(a);
                endpointSide.push_back(1);
            }
        }

        int E = endpoints.size();
        if (E == 0) break;

        int eBits = 0;
        while ((1 << eBits) < E + 1) eBits++;

        long long unknowns = 0;
        for (int v = 1; v <= N; v++) if (!placed[v]) unknowns++;
        if (unknowns == 0) break;

        long long phaseCost = (long long)eBits * (2LL * unknowns + E);
        if (opsUsed + phaseCost > opCap) break;

        vector<int> sig2(N + 1, 0);
        for (int b = 0; b < eBits; b++) {
            vector<int> ops;
            int transCount = 0;
            for (int i = 0; i < E; i++) {
                int idx1based = i + 1;
                int want = (idx1based >> b) & 1;
                if (want != inSet[endpoints[i]]) {
                    ops.push_back(endpoints[i]);
                    transCount++;
                }
            }
            for (int v = 1; v <= N; v++) {
                if (placed[v]) continue;
                ops.push_back(v);
                ops.push_back(v);
            }
            vector<int> resp = doQuery(ops);

            int idx = transCount;
            for (int v = 1; v <= N; v++) {
                if (placed[v]) continue;
                if (resp[idx] == 1) sig2[v] |= (1 << b);
                idx += 2;
            }
        }

        // Cleanup S to empty
        {
            vector<int> ops;
            for (int e : endpoints) if (inSet[e]) ops.push_back(e);
            if (!ops.empty()) doQuery(ops);
        }

        // Map sig2 -> endpoint claim. Each endpoint should be claimed by
        // exactly one unknown (its non-chain ring neighbor).
        vector<int> newNbr(E, 0);
        vector<int> claimsCount(E, 0);
        for (int v = 1; v <= N; v++) {
            if (placed[v]) continue;
            int s = sig2[v];
            if (s >= 1 && s <= E) {
                claimsCount[s - 1]++;
                newNbr[s - 1] = v;
            }
        }

        bool extended = false;
        for (int i = 0; i < E; i++) {
            if (claimsCount[i] != 1) continue;
            int v = newNbr[i];
            if (placed[v]) continue;
            int a = endpointChain[i];
            int side = endpointSide[i];
            if (side == 0) {
                chains[a].insert(chains[a].begin(), v);
            } else {
                chains[a].push_back(v);
            }
            placed[v] = 1;
            extended = true;
        }

        if (!extended) break;
    }

    // Output: anchor 1's chain, then remaining placed (deduped), then unplaced
    vector<int> result;
    vector<char> inResult(N + 1, 0);

    for (int v : chains[1]) {
        if (!inResult[v]) { result.push_back(v); inResult[v] = 1; }
    }
    for (int a = 2; a <= K; a++) {
        for (int v : chains[a]) {
            if (!inResult[v]) { result.push_back(v); inResult[v] = 1; }
        }
    }
    for (int v = 1; v <= N; v++) {
        if (!inResult[v]) { result.push_back(v); inResult[v] = 1; }
    }

    cout << -1;
    for (int x : result) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    return 0;
}
