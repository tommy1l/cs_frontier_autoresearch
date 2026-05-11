#include <bits/stdc++.h>
using namespace std;

// Doubler chain: N-1 doubler insts + 1 HALT = N insts, gives 2^N - 1 steps.
// Pre-push marker (c+1) (c in [0..N-2]) gives 2^N - 2^(c+1) + 1 steps,
// costs 1 extra inst. Padding pair adds 2 steps per 2 insts.
// Markers are 1..N (1-indexed to satisfy a,b >= 1).

int main() {
    long long k;
    cin >> k;

    if (k == 1) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    int bestN = -1, bestC = -2;
    long long bestPairs = 0, bestInst = 1LL << 30;

    for (int N = 2; N <= 31; N++) {
        for (int c = -1; c <= N - 2; c++) {
            long long base;
            if (c == -1) base = (1LL << N) - 1;
            else base = (1LL << N) - (1LL << (c + 1)) + 1;
            if (base > k) continue;
            long long padding = k - base;
            if (padding % 2 != 0) continue;
            long long pairs = padding / 2;
            long long setupInst = (c == -1) ? 0 : 1;
            long long inst = N + setupInst + 2 * pairs;
            if (inst > 512) continue;
            if (inst < bestInst) {
                bestInst = inst;
                bestN = N;
                bestC = c;
                bestPairs = pairs;
            }
        }
    }

    if (bestN < 0) {
        // Shouldn't happen for k <= 2^31-1, but emit a safe fallback.
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    long long n = bestInst;
    cout << n << "\n";
    int idx = 1;
    // Setup: pre-push marker (bestC+1) so doubler effectively skips lower levels.
    if (bestC >= 0) {
        // POP a never matched (stack empty here). PUSH (bestC+1), goto next.
        // POP a marker: use bestN+1 (unused). GOTO target if matched: any valid index (use 1).
        cout << "POP " << (bestN + 1) << " GOTO 1 PUSH " << (bestC + 1)
             << " GOTO " << (idx + 1) << "\n";
        idx++;
    }
    // Doubler chain: instructions idx .. idx+bestN-2 (bestN-1 of them).
    // Marker for level i is (i+1), so first marker = 1.
    int doublerStart = idx;
    for (int i = 0; i < bestN - 1; i++) {
        int nextInst = idx + i + 1;
        cout << "POP " << (i + 1) << " GOTO " << nextInst
             << " PUSH " << (i + 1) << " GOTO " << doublerStart << "\n";
    }
    idx += bestN - 1;
    // Padding pairs: each pair pushes/pops a marker that doesn't collide with doubler markers.
    // Use marker bestN+2 (unused). POP branches that aren't taken: GOTO 1 (always valid).
    int padMarker = bestN + 2;
    for (long long p = 0; p < bestPairs; p++) {
        // Push padMarker (stack empty -> else branch).
        cout << "POP " << (bestN + 3) << " GOTO 1 PUSH " << padMarker
             << " GOTO " << (idx + 1) << "\n";
        // Pop padMarker (always matches, else never taken).
        cout << "POP " << padMarker << " GOTO " << (idx + 2)
             << " PUSH 1 GOTO 1\n";
        idx += 2;
    }
    // HALT
    cout << "HALT PUSH 1 GOTO 1\n";
    return 0;
}
