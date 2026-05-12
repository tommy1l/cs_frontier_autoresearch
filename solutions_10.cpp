#include <bits/stdc++.h>
using namespace std;

int main() {
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        cin >> n;
        if (n == 1) {
            cout << "!" << "\n";
            cout.flush();
            continue;
        }
        vector<long long> d1(n+1, 0);
        for (int v = 2; v <= n; v++) {
            cout << "? 1 " << v << "\n";
            cout.flush();
            cin >> d1[v];
        }
        // Order vertices 2..n by d1 ascending
        vector<int> order;
        for (int v = 2; v <= n; v++) order.push_back(v);
        sort(order.begin(), order.end(), [&](int a, int b) {
            return d1[a] < d1[b];
        });

        vector<int> placed;
        placed.push_back(1);
        vector<tuple<int,int,long long>> edges;
        edges.reserve(n-1);

        for (int v : order) {
            // Find parent of v among placed vertices.
            // The parent is the deepest ancestor: an ancestor u satisfies
            // d(u,v) + d1[u] = d1[v]. We query placed u in decreasing d1 order.
            // We skip u=1 since 1 is always an ancestor; if no deeper one
            // found, parent = 1.
            vector<int> cand = placed;
            sort(cand.begin(), cand.end(), [&](int a, int b) {
                return d1[a] > d1[b];
            });
            int parent = -1;
            long long pw = 0;
            for (int u : cand) {
                if (u == 1) continue;
                cout << "? " << u << " " << v << "\n";
                cout.flush();
                long long duv;
                cin >> duv;
                if (duv + d1[u] == d1[v]) {
                    parent = u;
                    pw = duv;
                    break;
                }
            }
            if (parent == -1) {
                parent = 1;
                pw = d1[v];
            }
            edges.push_back({parent, v, pw});
            placed.push_back(v);
        }

        cout << "!";
        for (auto& [u, v, w] : edges) {
            cout << " " << u << " " << v << " " << w;
        }
        cout << "\n";
        cout.flush();
    }
    return 0;
}
