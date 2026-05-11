#include <bits/stdc++.h>
using namespace std;

// Multi-setup: pre-push markers at distinct levels in {2..N-1}.
// Doubler chain levels 1..N-1, HALT at end.
// Total steps = 2^N + 2*ks - 1 - sum 2^l   (l = level pre-pushed)
// + 2 * pairs from padding pairs.
// Cost: N + ks + 2*pairs instructions.

int main() {
    long long k;
    cin >> k;
    if (k == 1) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    long long bestCost = 1LL << 30;
    int bestN = -1, bestKs = -1, bestPairs = -1;
    long long bestM = 0;

    int minN = 1;
    while (((1LL << minN) - 1) < k) minN++;

    for (int N = max(minN, 2); N <= 31; N++) {
        if ((long long)N >= bestCost) break;
        long long maxM = (1LL << N) - 4;
        for (int pairs = 0; (long long)N + 2*pairs < bestCost; pairs++) {
            long long baseM = (1LL << N) - 1 - k + 2LL*pairs;
            if (baseM > maxM) break;
            for (int ks = 0; ks < N; ks++) {
                long long cost = (long long)N + ks + 2LL*pairs;
                if (cost >= bestCost) break;
                long long M = baseM + 2LL*ks;
                if (M > maxM) break;
                if (M < 0) continue;
                if (M & 3) continue;
                if (__builtin_popcountll(M) != ks) continue;
                bestCost = cost;
                bestN = N;
                bestKs = ks;
                bestPairs = pairs;
                bestM = M;
                break;
            }
        }
    }

    if (bestN < 0) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    int N = bestN, ks = bestKs, pairs = bestPairs;
    long long M = bestM;
    vector<int> levels;
    for (int b = 2; b < N; b++) {
        if (M & (1LL << b)) levels.push_back(b);
    }
    sort(levels.rbegin(), levels.rend());

    int total = N + ks + 2 * pairs;
    cout << total << "\n";
    int idx = 1;
    int unused = N + 1;
    for (int l : levels) {
        cout << "POP " << unused << " GOTO 1 PUSH " << l << " GOTO " << (idx + 1) << "\n";
        idx++;
    }
    int dStart = idx;
    for (int i = 0; i < N - 1; i++) {
        int m = i + 1;
        cout << "POP " << m << " GOTO " << (idx + 1)
             << " PUSH " << m << " GOTO " << dStart << "\n";
        idx++;
    }
    int padMarker = N + 2;
    int padUnused = N + 3;
    for (int p = 0; p < pairs; p++) {
        cout << "POP " << padUnused << " GOTO 1 PUSH " << padMarker
             << " GOTO " << (idx + 1) << "\n";
        cout << "POP " << padMarker << " GOTO " << (idx + 2)
             << " PUSH 1 GOTO 1\n";
        idx += 2;
    }
    cout << "HALT PUSH 1 GOTO 1\n";
    return 0;
}
