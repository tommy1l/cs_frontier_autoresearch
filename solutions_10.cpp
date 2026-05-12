#include <bits/stdc++.h>
using namespace std;

int main() {
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        cin >> n;
        if (n == 1) {
            cout << "!\n";
            cout.flush();
            continue;
        }
        vector<long long> d1(n+1, 0);
        for (int v = 2; v <= n; v++) {
            cout << "? 1 " << v << "\n";
            cout.flush();
            cin >> d1[v];
        }
        vector<int> order;
        order.reserve(n-1);
        for (int v = 2; v <= n; v++) order.push_back(v);
        sort(order.begin(), order.end(), [&](int a, int b) {
            return d1[a] < d1[b];
        });

        vector<int> placed_parent(n+1, 0);
        vector<long long> placed_pw(n+1, 0);
        vector<int> elim_v(n+1, -1);
        int v_id = 0;

        vector<tuple<int,int,long long>> edges;
        edges.reserve(n-1);

        vector<int> placed;
        placed.reserve(n);
        placed.push_back(1);  // ascending d order: d1[1]=0

        for (int v : order) {
            v_id++;
            long long Dv = d1[v];
            int parent = -1;
            long long pw = 0;
            long long min_pd = 0;

            // Iterate placed in descending d order (placed is ascending d)
            for (int i = (int)placed.size() - 1; i >= 0; i--) {
                int u = placed[i];
                if (d1[u] >= Dv) continue;        // not a valid ancestor (same depth or larger)
                if (u == 1) continue;             // root handled as fallback
                if (d1[u] < min_pd) break;        // below depth lower-bound; iteration is desc
                if (elim_v[u] == v_id) continue;  // eliminated for this v

                cout << "? " << u << " " << v << "\n";
                cout.flush();
                long long duv;
                cin >> duv;

                if (duv + d1[u] == Dv) {
                    // u is ancestor; deepest ancestor (since we iterate desc) = parent
                    parent = u;
                    pw = duv;
                    break;
                }

                // u not ancestor. LCA depth on path v..root.
                long long Ld = (Dv + d1[u] - duv) / 2;
                if (Ld > min_pd) min_pd = Ld;

                // Chain elimination: u..(child of L on path to u) cannot be ancestors of v
                int cur = u;
                while (cur != 0 && d1[cur] > Ld) {
                    elim_v[cur] = v_id;
                    cur = placed_parent[cur];
                }
            }

            if (parent == -1) {
                parent = 1;
                pw = Dv;
            }
            edges.push_back({parent, v, pw});
            placed_parent[v] = parent;
            placed_pw[v] = pw;
            placed.push_back(v);
        }

        cout << "!";
        for (auto& [u, vv, w] : edges) {
            cout << " " << u << " " << vv << " " << w;
        }
        cout << "\n";
        cout.flush();
    }
    return 0;
}
