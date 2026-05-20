#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h, R, F;
    int mnx, mny;
    vector<pair<int,int>> cells;
    vector<int> col_min_y; // for each x in [0,w), minimum cell.y in that column (INT_MAX if none)
};

int main(){
    int n;
    if(scanf("%d", &n) != 1) return 0;
    vector<vector<pair<int,int>>> P(n);
    long long T = 0;
    for(int i = 0; i < n; i++){
        int k;
        scanf("%d", &k);
        P[i].resize(k);
        T += k;
        for(int j = 0; j < k; j++){
            scanf("%d %d", &P[i][j].first, &P[i][j].second);
        }
    }

    auto apply_transform = [](const vector<pair<int,int>>& p, int R, int F) {
        vector<pair<int,int>> out(p.size());
        for(size_t i = 0; i < p.size(); i++){
            int x = p[i].first, y = p[i].second;
            if(F) x = -x;
            for(int r = 0; r < R; r++){
                int nx = y, ny = -x;
                x = nx; y = ny;
            }
            out[i] = {x, y};
        }
        int mnx = INT_MAX, mny = INT_MAX;
        for(auto& pr : out){
            mnx = min(mnx, pr.first);
            mny = min(mny, pr.second);
        }
        int omnx = mnx, omny = mny;
        for(auto& pr : out){
            pr.first -= mnx;
            pr.second -= mny;
        }
        return make_tuple(out, omnx, omny);
    };

    vector<vector<Orient>> ori(n);
    for(int i = 0; i < n; i++){
        set<vector<pair<int,int>>> seen;
        for(int F = 0; F < 2; F++){
            for(int R = 0; R < 4; R++){
                auto tup = apply_transform(P[i], R, F);
                auto& c = get<0>(tup);
                int mx = get<1>(tup), my = get<2>(tup);
                auto c_sorted = c;
                sort(c_sorted.begin(), c_sorted.end());
                if(seen.count(c_sorted)) continue;
                seen.insert(c_sorted);
                int w = 0, h = 0;
                for(auto& pr : c){
                    w = max(w, pr.first + 1);
                    h = max(h, pr.second + 1);
                }
                Orient o;
                o.w = w; o.h = h; o.R = R; o.F = F;
                o.mnx = mx; o.mny = my;
                o.cells = c;
                o.col_min_y.assign(w, INT_MAX);
                for(auto& pr : c){
                    if(pr.second < o.col_min_y[pr.first]) o.col_min_y[pr.first] = pr.second;
                }
                ori[i].push_back(o);
            }
        }
    }

    // Sort pieces by cell count desc, then max dim desc
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b){
        if(P[a].size() != P[b].size()) return P[a].size() > P[b].size();
        int ma = 0, mb = 0;
        for(auto& o : ori[a]) ma = max(ma, max(o.w, o.h));
        for(auto& o : ori[b]) mb = max(mb, max(o.w, o.h));
        return ma > mb;
    });

    auto try_pack = [&](int S, vector<tuple<int,int,int,int>>& placements) -> bool {
        vector<int> skyline(S, 0);
        vector<vector<char>> grid(S, vector<char>(S, 0));
        placements.assign(n, make_tuple(0,0,0,0));

        for(int idx : order){
            int best_y = INT_MAX;
            long long best_shadow = LLONG_MAX;
            int best_x = 0, best_orient = 0;
            for(int oi = 0; oi < (int)ori[idx].size(); oi++){
                const auto& o = ori[idx][oi];
                if(o.w > S || o.h > S) continue;
                for(int X = 0; X + o.w <= S; X++){
                    int Y_lb = 0;
                    for(const auto& cell : o.cells){
                        int req = skyline[X + cell.first] - cell.second;
                        if(req > Y_lb) Y_lb = req;
                    }
                    if(Y_lb < 0) Y_lb = 0;
                    if(Y_lb + o.h > S) continue;

                    int Y = Y_lb;
                    bool found = false;
                    while(Y + o.h <= S){
                        bool ok = true;
                        for(const auto& cell : o.cells){
                            if(grid[X + cell.first][Y + cell.second]){
                                ok = false; break;
                            }
                        }
                        if(ok){ found = true; break; }
                        Y++;
                    }
                    if(!found) continue;

                    // Compute shadow: empty cells trapped beneath this placement, per piece column
                    long long shadow = 0;
                    for(int c = 0; c < o.w; c++){
                        if(o.col_min_y[c] == INT_MAX) continue;
                        int bottom = Y + o.col_min_y[c];
                        int sky = skyline[X + c];
                        if(bottom > sky) shadow += (bottom - sky);
                    }

                    bool better = false;
                    if(Y < best_y) better = true;
                    else if(Y == best_y){
                        if(shadow < best_shadow) better = true;
                        else if(shadow == best_shadow && X < best_x) better = true;
                    }
                    if(better){
                        best_y = Y;
                        best_shadow = shadow;
                        best_x = X;
                        best_orient = oi;
                    }
                }
            }
            if(best_y == INT_MAX) return false;
            const auto& o = ori[idx][best_orient];
            int X = best_x, Y = best_y;
            for(const auto& cell : o.cells){
                grid[X + cell.first][Y + cell.second] = 1;
                int top = Y + cell.second + 1;
                if(top > skyline[X + cell.first]) skyline[X + cell.first] = top;
            }
            placements[idx] = make_tuple(X - o.mnx, Y - o.mny, o.R, o.F);
        }
        return true;
    };

    int Smin = max(10, (int)ceil(sqrt((double)T)));
    int S = Smin;
    vector<tuple<int,int,int,int>> placements;
    while(true){
        if(try_pack(S, placements)) break;
        S++;
        if(S > Smin * 3 + 20){
            try_pack(S, placements);
            break;
        }
    }

    printf("%d %d\n", S, S);
    for(int i = 0; i < n; i++){
        int X = get<0>(placements[i]);
        int Y = get<1>(placements[i]);
        int R = get<2>(placements[i]);
        int F = get<3>(placements[i]);
        printf("%d %d %d %d\n", X, Y, R, F);
    }
    return 0;
}
