#include <bits/stdc++.h>
using namespace std;

map<tuple<int,int,int>, int> mem;
vector<vector<pair<int,int>>> edges;

// Build state for length-B suffix in [Lo, Hi]
int build(int B, int Lo, int Hi) {
    auto key = make_tuple(B, Lo, Hi);
    auto it = mem.find(key);
    if (it != mem.end()) return it->second;
    int sid = (int)edges.size();
    mem[key] = sid;
    edges.push_back({});
    if (B == 0) return sid;
    // Find longest common high-bit prefix of Lo, Hi.
    int diff = Lo ^ Hi;
    int common = 0;
    if (diff == 0) {
        common = B;
    } else {
        int hb = 31 - __builtin_clz(diff);
        common = B - 1 - hb;
    }
    if (common == B) {
        // Lo == Hi, single value: follow fixed bits
        int cur = sid;
        for (int i = B - 1; i >= 0; i--) {
            int b = (Lo >> i) & 1;
            int child;
            if (i == 0) {
                child = build(0, 0, 0);
            } else {
                // create or reuse single-value state for the remaining bits
                int suffix = Lo & ((1 << i) - 1);
                child = build(i, suffix, suffix);
            }
            edges[cur].push_back({child, b});
            cur = child;
            break; // only one step needed; recursion handles rest
        }
        return sid;
    }
    // Emit common prefix as a chain of single-edge nodes
    int cur = sid;
    for (int i = 0; i < common; i++) {
        int b = (Lo >> (B - 1 - i)) & 1;
        int restB = B - 1 - i;
        int restLo = Lo & ((1 << restB) - 1);
        int restHi = Hi & ((1 << restB) - 1);
        // The remaining suffix has full range from low (Lo's bits) to high (Hi's bits)
        // since common bits match, restLo = Lo mod 2^restB, restHi = Hi mod 2^restB
        // After common prefix, the differing bit positions start.
        int child;
        if (i == common - 1) {
            child = build(restB, restLo, restHi);
        } else {
            // Skip ahead: collapse common chain into single recursion
            child = build(restB, restLo, restHi);
            edges[cur].push_back({child, b});
            return sid;
        }
        edges[cur].push_back({child, b});
        cur = child;
    }
    if (common > 0) return sid;

    // common == 0: split on top bit
    int mid = 1 << (B - 1);
    int Lo0 = Lo, Hi0 = min(Hi, mid - 1);
    if (Lo0 <= Hi0) {
        int child = build(B - 1, Lo0, Hi0);
        edges[sid].push_back({child, 0});
    }
    if (Hi >= mid) {
        int Lo1 = max(Lo, mid) - mid;
        int Hi1 = Hi - mid;
        int child = build(B - 1, Lo1, Hi1);
        edges[sid].push_back({child, 1});
    }
    return sid;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int L, R;
    cin >> L >> R;

    int bL = 0, bR = 0;
    for (int x = L; x; x >>= 1) bL++;
    for (int x = R; x; x >>= 1) bR++;

    int start = (int)edges.size();
    edges.push_back({});

    if (bL == bR) {
        int B = bL;
        int mid = 1 << (B - 1);
        int child = build(B - 1, L - mid, R - mid);
        edges[start].push_back({child, 1});
    } else {
        int midL = 1 << (bL - 1);
        int childL = build(bL - 1, L - midL, midL - 1);
        edges[start].push_back({childL, 1});
        for (int B = bL + 1; B < bR; B++) {
            int childM = build(B - 1, 0, (1 << (B-1)) - 1);
            edges[start].push_back({childM, 1});
        }
        int midR = 1 << (bR - 1);
        int childR = build(bR - 1, 0, R - midR);
        edges[start].push_back({childR, 1});
    }

    int n = edges.size();
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << edges[i].size();
        for (auto& e : edges[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
