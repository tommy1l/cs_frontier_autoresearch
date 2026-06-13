#include <bits/stdc++.h>
using namespace std;

int n;

int do_query(const vector<int>& q) {
    cout << 0;
    for (int x : q) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    int x; cin >> x;
    return x;
}

void output_answer(const vector<int>& p) {
    cout << 1;
    for (int x : p) cout << ' ' << x;
    cout << '\n';
    cout.flush();
}

int main() {
    cin >> n;

    if (n == 1) {
        output_answer({1});
        return 0;
    }

    if (n == 2) {
        vector<int> q = {1, 2};
        int ans = do_query(q);
        if (ans == 2) output_answer({1, 2});
        else output_answer({2, 1});
        return 0;
    }

    vector<int> p(n + 1, 0);

    // Phase 1: linear scan to find pos_of_1 and pos_of_2.
    // Q_i = (1,1,...,1, 2 at position i, 1,1,...,1).
    // Ans = [pos_of_1 != i] + [p[i] = 2] = 1 - [p[i]=1] + [p[i]=2].
    //   p[i]=1 -> 0 (identifies pos_of_1)
    //   p[i]=2 -> 2 (identifies pos_of_2)
    //   else   -> 1
    int pos_of_1 = -1, pos_of_2 = -1;
    for (int i = 1; i <= n; i++) {
        if (pos_of_1 != -1 && pos_of_2 != -1) break;
        vector<int> q(n, 1);
        q[i - 1] = 2;
        int ans = do_query(q);
        if (ans == 0) pos_of_1 = i;
        else if (ans == 2) pos_of_2 = i;
    }

    p[pos_of_1] = 1;
    p[pos_of_2] = 2;

    vector<int> unknown_positions;
    for (int i = 1; i <= n; i++) if (p[i] == 0) unknown_positions.push_back(i);

    // Phase 2: For each value v = 3..n-1, BS within unknown_positions.
    // Query Q where Q[j] = v for j in T, Q[j] = 1 for j not in T.
    // - pos_of_1: Q=1, p=1, match -> +1.
    // - pos_of_v' (v'=2..v-1): Q=1, p=v'>=2, no match.
    // - j in T (unknown): Q=v, match iff p[j]=v -> contributes [pos_of_v in T].
    // - j not in T (unknown): Q=1, p[j]>=v>1, no match.
    // Total = 1 + [pos_of_v in T].
    for (int v = 3; v <= n - 1; v++) {
        int lo = 0, hi = (int)unknown_positions.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> q(n, 1);
            for (int k = lo; k <= mid; k++) q[unknown_positions[k] - 1] = v;
            int ans = do_query(q);
            if (ans == 2) hi = mid;
            else lo = mid + 1;
        }
        p[unknown_positions[lo]] = v;
        unknown_positions.erase(unknown_positions.begin() + lo);
    }

    if (!unknown_positions.empty()) {
        p[unknown_positions[0]] = n;
    }

    vector<int> ans_perm;
    for (int i = 1; i <= n; i++) ans_perm.push_back(p[i]);
    output_answer(ans_perm);

    return 0;
}
