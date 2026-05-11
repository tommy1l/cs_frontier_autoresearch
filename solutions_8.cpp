#include <bits/stdc++.h>
using namespace std;

// Doubler chain: (N-1) doubler insts + HALT = N insts, base 2^N - 1 steps.
// Pre-push p distinct markers at levels m_1 > m_2 > ... > m_p in [2, N-1].
// Pushed largest first, so smallest is on top — each marker m saves
// 2^m - 2 steps relative to base (1 inst cost). Padding pairs (2 insts,
// 2 steps each) fill the remainder. Total steps = 2^N - sum(2^m_i)
// + 2p - 1 + 2*pairs; total insts = N + p + 2*pairs.

int main() {
    long long k;
    cin >> k;

    if (k == 1) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    long long bestInsts = LLONG_MAX;
    int bestN = -1;
    vector<int> bestMarkers;
    long long bestPairs = 0;

    int Nmin = 1;
    while ((1LL << Nmin) - 1 < k) Nmin++;

    for (int N = Nmin; N <= 32; N++) {
        long long base = (1LL << N) - 1;
        long long D = base - k;
        if (D < 0) continue;
        vector<int> markers;
        long long rem = D;
        int mMax = min(N - 1, 30);
        for (int m = mMax; m >= 2; m--) {
            long long save = (1LL << m) - 2;
            if (save <= rem) {
                markers.push_back(m);
                rem -= save;
            }
        }
        if (rem % 2 != 0) continue;
        long long pairs = rem / 2;
        long long insts = (long long)N + (long long)markers.size() + 2 * pairs;
        if (insts > 512) continue;
        if (insts < bestInsts) {
            bestInsts = insts;
            bestN = N;
            bestMarkers = markers;
            bestPairs = pairs;
        }
    }

    if (bestN < 0) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }

    int N = bestN;
    cout << bestInsts << "\n";
    int idx = 1;

    int preX = N + 1; // never on stack
    for (int m : bestMarkers) {
        cout << "POP " << preX << " GOTO 1 PUSH " << m
             << " GOTO " << (idx + 1) << "\n";
        idx++;
    }

    int doublerStart = idx;
    for (int i = 0; i < N - 1; i++) {
        int nextInst = idx + i + 1;
        cout << "POP " << (i + 1) << " GOTO " << nextInst
             << " PUSH " << (i + 1) << " GOTO " << doublerStart << "\n";
    }
    idx += N - 1;

    int padMarker = N + 2;
    int padPopX = N + 3;
    for (long long pp = 0; pp < bestPairs; pp++) {
        cout << "POP " << padPopX << " GOTO 1 PUSH " << padMarker
             << " GOTO " << (idx + 1) << "\n";
        cout << "POP " << padMarker << " GOTO " << (idx + 2)
             << " PUSH 1 GOTO 1\n";
        idx += 2;
    }

    cout << "HALT PUSH 1 GOTO 1\n";
    return 0;
}
