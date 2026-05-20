#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h, R, F;
    int mnx, mny;
    vector<pair<int,int>> cells;
    vector<int> col_min_y;
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

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b){
        if(P[a].size() != P[b].size()) return P[a].size() > P[b].size();
        int ma = 0, mb = 0;
        for(auto& o : ori[a]) ma = max(ma, max(o.w, o.h));
        for(auto& o : ori[b]) mb = max(mb, max(o.w, o.h));
        return ma > mb;
    });

    int S_max = (int)ceil(sqrt(2.5 * (double)T)) + 25;
    if(S_max < 32) S_max = 32;

    vector<int> skyline(S_max, 0);
    vector<vector<char>> grid(S_max, vector<char>(S_max, 0));
    int cur_max_x = 0, cur_max_y = 0;
    vector<tuple<int,int,int,int>> placements(n);

    for(int idx : order){
        int best_new_side = INT_MAX;
        long long best_shadow = LLONG_MAX;
        int best_Y = INT_MAX, best_X = 0, best_oi = 0;

        for(int oi = 0; oi < (int)ori[idx].size(); oi++){
            const auto& o = ori[idx][oi];
            if(o.w > S_max || o.h > S_max) continue;

            int X_limit = S_max - o.w;
            if(best_new_side != INT_MAX){
                int xlim2 = best_new_side - o.w;
                if(xlim2 < X_limit) X_limit = xlim2;
            }
            for(int X = 0; X <= X_limit; X++){
                int Y_lb = 0;
                for(const auto& c : o.cells){
                    int req = skyline[X + c.first] - c.second;
                    if(req > Y_lb) Y_lb = req;
                }
                if(Y_lb < 0) Y_lb = 0;
                if(Y_lb + o.h > S_max) continue;

                if(best_new_side != INT_MAX){
                    int ymax_min = Y_lb + o.h;
                    int ny_lb = max(cur_max_y, ymax_min);
                    int nx = max(cur_max_x, X + o.w);
                    int ns_lb = max(nx, ny_lb);
                    if(ns_lb > best_new_side) continue;
                }

                int Y = Y_lb;
                bool found = false;
                while(Y + o.h <= S_max){
                    bool ok = true;
                    for(const auto& c : o.cells){
                        if(grid[X + c.first][Y + c.second]){
                            ok = false; break;
                        }
                    }
                    if(ok){ found = true; break; }
                    Y++;
                }
                if(!found) continue;

                int new_max_x = max(cur_max_x, X + o.w);
                int new_max_y = max(cur_max_y, Y + o.h);
                int new_side = max(new_max_x, new_max_y);
                if(new_side > best_new_side) continue;

                long long shadow = 0;
                for(int c = 0; c < o.w; c++){
                    if(o.col_min_y[c] == INT_MAX) continue;
                    int bottom = Y + o.col_min_y[c];
                    int sky = skyline[X + c];
                    if(bottom > sky) shadow += (bottom - sky);
                }

                bool better = false;
                if(new_side < best_new_side) better = true;
                else if(new_side == best_new_side){
                    if(shadow < best_shadow) better = true;
                    else if(shadow == best_shadow){
                        if(Y < best_Y) better = true;
                        else if(Y == best_Y && X < best_X) better = true;
                    }
                }
                if(better){
                    best_new_side = new_side;
                    best_shadow = shadow;
                    best_Y = Y; best_X = X; best_oi = oi;
                }
            }
        }

        if(best_new_side == INT_MAX){
            // Fall back: place at (0, top of all skylines) - shouldn't normally happen
            int Y = 0;
            for(int x = 0; x < S_max; x++) Y = max(Y, skyline[x]);
            best_oi = 0; best_X = 0; best_Y = Y;
        }

        const auto& o = ori[idx][best_oi];
        int X = best_X, Y = best_Y;
        for(const auto& c : o.cells){
            grid[X + c.first][Y + c.second] = 1;
            int top = Y + c.second + 1;
            if(top > skyline[X + c.first]) skyline[X + c.first] = top;
        }
        if(X + o.w > cur_max_x) cur_max_x = X + o.w;
        if(Y + o.h > cur_max_y) cur_max_y = Y + o.h;
        placements[idx] = make_tuple(X - o.mnx, Y - o.mny, o.R, o.F);
    }

    int side = max(cur_max_x, cur_max_y);
    printf("%d %d\n", side, side);
    for(int i = 0; i < n; i++){
        int X = get<0>(placements[i]);
        int Y = get<1>(placements[i]);
        int R = get<2>(placements[i]);
        int F = get<3>(placements[i]);
        printf("%d %d %d %d\n", X, Y, R, F);
    }
    return 0;
}
