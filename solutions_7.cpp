#include <bits/stdc++.h>
using namespace std;

// Iter 6: bottom-up DAFSA construction.

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int L, R;
    cin >> L >> R;

    int bL = 0, bR = 0;
    for (int x = L; x; x >>= 1) bL++;
    for (int x = R; x; x >>= 1) bR++;

    vector<set<pair<int,int>>> intervals(bR + 1);

    if (bL == bR) {
        int B = bL;
        int mid = 1 << (B - 1);
        intervals[B - 1].insert({L - mid, R - mid});
    } else {
        int midL = 1 << (bL - 1);
        intervals[bL - 1].insert({L - midL, midL - 1});
        for (int B = bL + 1; B < bR; B++) {
            intervals[B - 1].insert({0, (1 << (B-1)) - 1});
        }
        int midR = 1 << (bR - 1);
        intervals[bR - 1].insert({0, R - midR});
    }

    for (int B = bR - 1; B >= 1; B--) {
        for (auto& [Lo, Hi] : intervals[B]) {
            int mid = 1 << (B - 1);
            int Lo0 = Lo, Hi0 = min(Hi, mid - 1);
            if (Lo0 <= Hi0) intervals[B - 1].insert({Lo0, Hi0});
            if (Hi >= mid) {
                int Lo1 = max(Lo, mid) - mid;
                int Hi1 = Hi - mid;
                intervals[B - 1].insert({Lo1, Hi1});
            }
        }
    }

    map<tuple<int,int,int>, int> id;
    int nextId = 1;
    for (int B = 0; B <= bR - 1; B++) {
        for (auto& [Lo, Hi] : intervals[B]) {
            id[{B, Lo, Hi}] = nextId++;
        }
    }
    int total = nextId;

    vector<vector<pair<int,int>>> edges(total);

    auto getId = [&](int B, int Lo, int Hi) {
        return id[{B, Lo, Hi}];
    };

    if (bL == bR) {
        int B = bL;
        int mid = 1 << (B - 1);
        int child = getId(B - 1, L - mid, R - mid);
        edges[0].push_back({child, 1});
    } else {
        int midL = 1 << (bL - 1);
        int childL = getId(bL - 1, L - midL, midL - 1);
        edges[0].push_back({childL, 1});
        for (int B = bL + 1; B < bR; B++) {
            int childM = getId(B - 1, 0, (1 << (B-1)) - 1);
            edges[0].push_back({childM, 1});
        }
        int midR = 1 << (bR - 1);
        int childR = getId(bR - 1, 0, R - midR);
        edges[0].push_back({childR, 1});
    }

    for (int B = bR - 1; B >= 1; B--) {
        for (auto& [Lo, Hi] : intervals[B]) {
            int sid = getId(B, Lo, Hi);
            int mid = 1 << (B - 1);
            int Lo0 = Lo, Hi0 = min(Hi, mid - 1);
            if (Lo0 <= Hi0) {
                int child = getId(B - 1, Lo0, Hi0);
                edges[sid].push_back({child, 0});
            }
            if (Hi >= mid) {
                int Lo1 = max(Lo, mid) - mid;
                int Hi1 = Hi - mid;
                int child = getId(B - 1, Lo1, Hi1);
                edges[sid].push_back({child, 1});
            }
        }
    }

    cout << total << "\n";
    for (int i = 0; i < total; i++) {
        cout << edges[i].size();
        for (auto& e : edges[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
