#include <bits/stdc++.h>
using namespace std;

// Iter 1: ping-pong construction. For k = 2m+1, use 2m instructions.
// Inst 1: "POP 1 GOTO 2 PUSH 1 GOTO 1" -- on empty, push 1 and loop; on [1], pop and goto 2.
// Inst 2: "POP 99 GOTO 99 PUSH 2 GOTO 3" -- pushes 2.
// Inst 3: "POP 2 GOTO 4 PUSH 1 GOTO 1" -- pops 2.
// ...
// Inst 2m: HALT.

int main() {
    long long k;
    cin >> k;
    if (k == 1) {
        cout << "1\nHALT PUSH 1 GOTO 1\n";
        return 0;
    }
    long long m = (k - 1) / 2;
    long long n = 2 * m;
    if (n > 512) {
        // Fallback - this won't be valid, but it's a placeholder.
        n = 512;
    }
    cout << n << "\n";
    // inst 1: dual push (when empty) / pop (when [1]).
    cout << "POP 1 GOTO 2 PUSH 1 GOTO 1\n";
    // inst 2..2m-1: alternate push and pop using markers 2, 3, ..., m+1.
    int marker = 2;
    for (long long i = 2; i <= n - 1; i++) {
        if (i % 2 == 0) {
            // push marker
            cout << "POP 99 GOTO " << n << " PUSH " << marker << " GOTO " << (i + 1) << "\n";
        } else {
            // pop marker
            cout << "POP " << marker << " GOTO " << (i + 1) << " PUSH 1 GOTO 1\n";
            marker++;
        }
    }
    cout << "HALT PUSH 1 GOTO 1\n";
    return 0;
}
