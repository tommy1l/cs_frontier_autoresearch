#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h, R, F;
    int mnx, mny;
    vector<pair<int,int>> cells;
    vector<int> col_min_y;
    vector<int> col_max_y; // -1 if no cell in column
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
                o.col_max_y.assign(w, -1);
                for(auto& pr : c){
                    if(pr.second < o.col_min_y[pr.first]) o.col_min_y[pr.first] = pr.second;
                    if(pr.second > o.col_max_y[pr.first]) o.col_max_y[pr.first] = pr.second;
                }
                ori[i].push_back(o);
            }
        }
    }

    auto try_pack = [&](const vector<int>& order, int S, vector<tuple<int,int,int,int>>& placements) -> bool {
        vector<int> skyline(S, 0);
        vector<vector<char>> grid(S, vector<char>(S, 0));
        placements.assign(n, make_tuple(0,0,0,0));

        for(int idx : order){
            int best_top = INT_MAX;
            long long best_combined = LLONG_MAX;
            int best_y_chosen = INT_MAX;
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
                    if(Y_lb + o.h > best_top) continue;

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
                    int top = Y + o.h;
                    if(top > best_top) continue;

                    long long shadow = 0;
                    for(int c = 0; c < o.w; c++){
                        if(o.col_min_y[c] == INT_MAX) continue;
                        int bottom = Y + o.col_min_y[c];
                        int sky = skyline[X + c];
                        if(bottom > sky) shadow += (bottom - sky);
                    }

                    // Compute jaggedness delta in piece footprint + 1 boundary col on each side
                    int lo = (X > 0) ? X - 1 : 0;
                    int hi = (X + o.w < S) ? X + o.w + 1 : S;
                    long long jag_delta = 0;
                    for(int c = lo; c + 1 < hi; c++){
                        int b1 = skyline[c];
                        int b2 = skyline[c + 1];
                        int a1 = b1, a2 = b2;
                        if(c >= X && c < X + o.w && o.col_max_y[c - X] >= 0){
                            int t = Y + o.col_max_y[c - X] + 1;
                            if(t > a1) a1 = t;
                        }
                        if(c + 1 >= X && c + 1 < X + o.w && o.col_max_y[c + 1 - X] >= 0){
                            int t = Y + o.col_max_y[c + 1 - X] + 1;
                            if(t > a2) a2 = t;
                        }
                        jag_delta += (long long)abs(a1 - a2) - abs(b1 - b2);
                    }

                    long long combined = shadow + jag_delta;
                    bool better = false;
                    if(top < best_top) better = true;
                    else if(top == best_top){
                        if(combined < best_combined) better = true;
                        else if(combined == best_combined){
                            if(Y < best_y_chosen) better = true;
                            else if(Y == best_y_chosen && X < best_x) better = true;
                        }
                    }
                    if(better){
                        best_top = top;
                        best_combined = combined;
                        best_y_chosen = Y;
                        best_x = X;
                        best_orient = oi;
                    }
                }
            }
            if(best_top == INT_MAX) return false;
            const auto& o = ori[idx][best_orient];
            int X = best_x, Y = best_y_chosen;
            for(const auto& cell : o.cells){
                grid[X + cell.first][Y + cell.second] = 1;
                int top = Y + cell.second + 1;
                if(top > skyline[X + cell.first]) skyline[X + cell.first] = top;
            }
            placements[idx] = make_tuple(X - o.mnx, Y - o.mny, o.R, o.F);
        }
        return true;
    };

    // Build several orderings
    vector<int> base(n);
    iota(base.begin(), base.end(), 0);

    auto max_dim = [&](int i){
        int m = 0;
        for(auto& o : ori[i]) m = max(m, max(o.w, o.h));
        return m;
    };
    auto min_dim = [&](int i){
        int m = INT_MAX;
        for(auto& o : ori[i]) m = min(m, min(o.w, o.h));
        return m;
    };

    vector<vector<int>> orderings;

    // Ordering 1: cell count desc, max dim desc
    {
        auto o = base;
        sort(o.begin(), o.end(), [&](int a, int b){
            if(P[a].size() != P[b].size()) return P[a].size() > P[b].size();
            return max_dim(a) > max_dim(b);
        });
        orderings.push_back(o);
    }
    // Ordering 2: max dim desc, cell count desc
    {
        auto o = base;
        sort(o.begin(), o.end(), [&](int a, int b){
            int ma = max_dim(a), mb = max_dim(b);
            if(ma != mb) return ma > mb;
            return P[a].size() > P[b].size();
        });
        orderings.push_back(o);
    }
    // Ordering 3: min dim desc (squarest first), cell count desc
    {
        auto o = base;
        sort(o.begin(), o.end(), [&](int a, int b){
            int ma = min_dim(a), mb = min_dim(b);
            if(ma != mb) return ma > mb;
            return P[a].size() > P[b].size();
        });
        orderings.push_back(o);
    }

    int Smin = max(10, (int)ceil(sqrt((double)T)));

    int best_S = INT_MAX;
    vector<tuple<int,int,int,int>> best_placements;
    for(const auto& ord : orderings){
        int S = Smin;
        vector<tuple<int,int,int,int>> placements;
        while(true){
            if(S >= best_S) break;
            if(try_pack(ord, S, placements)){
                if(S < best_S){
                    best_S = S;
                    best_placements = placements;
                }
                break;
            }
            S++;
            if(S > Smin * 3 + 20){
                try_pack(ord, S, placements);
                if(S < best_S){
                    best_S = S;
                    best_placements = placements;
                }
                break;
            }
        }
    }

    printf("%d %d\n", best_S, best_S);
    for(int i = 0; i < n; i++){
        int X = get<0>(best_placements[i]);
        int Y = get<1>(best_placements[i]);
        int R = get<2>(best_placements[i]);
        int F = get<3>(best_placements[i]);
        printf("%d %d %d %d\n", X, Y, R, F);
    }
    return 0;
}
