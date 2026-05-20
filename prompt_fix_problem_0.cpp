#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h, R, F;
    int mnx, mny; // mins of transformed cells before normalization
    vector<pair<int,int>> cells; // normalized to start at (0,0)
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
        for(int F = 0; F < 2; F++){
            for(int R = 0; R < 4; R++){
                auto tup = apply_transform(P[i], R, F);
                auto& c = get<0>(tup);
                int mx = get<1>(tup), my = get<2>(tup);
                int w = 0, h = 0;
                for(auto& pr : c){
                    w = max(w, pr.first + 1);
                    h = max(h, pr.second + 1);
                }
                Orient o;
                o.w = w; o.h = h; o.R = R; o.F = F;
                o.mnx = mx; o.mny = my;
                o.cells = move(c);
                ori[i].push_back(o);
            }
        }
    }

    // Pick orientation minimizing height (ties: smaller width)
    vector<int> pick(n);
    for(int i = 0; i < n; i++){
        int best = 0;
        for(int j = 1; j < 8; j++){
            const auto& a = ori[i][j];
            const auto& b = ori[i][best];
            if(a.h < b.h || (a.h == b.h && a.w < b.w)) best = j;
        }
        pick[i] = best;
    }

    // Sort pieces by height desc, then width desc
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b){
        const auto& A = ori[a][pick[a]];
        const auto& B = ori[b][pick[b]];
        if(A.h != B.h) return A.h > B.h;
        return A.w > B.w;
    });

    int max_w = 0;
    for(int i = 0; i < n; i++){
        max_w = max(max_w, ori[i][pick[i]].w);
    }

    int Smin = max(max_w, (int)ceil(sqrt((double)T)));
    int Smax = 4 * Smin + 20;

    long long best_area = LLONG_MAX;
    int best_S = Smin;
    for(int S = Smin; S <= Smax; S++){
        int curX = 0, curY = 0, shelfH = 0;
        for(int idx : order){
            const auto& o = ori[idx][pick[idx]];
            if(curX + o.w > S){
                curY += shelfH;
                curX = 0;
                shelfH = 0;
            }
            curX += o.w;
            shelfH = max(shelfH, o.h);
        }
        int H = curY + shelfH;
        int side = max(S, H);
        long long area = (long long)side * side;
        if(area < best_area){
            best_area = area;
            best_S = S;
        }
    }

    int S = best_S;
    int curX = 0, curY = 0, shelfH = 0;
    vector<tuple<int,int,int,int>> place(n);
    for(int idx : order){
        const auto& o = ori[idx][pick[idx]];
        if(curX + o.w > S){
            curY += shelfH;
            curX = 0;
            shelfH = 0;
        }
        int X = curX - o.mnx;
        int Y = curY - o.mny;
        place[idx] = make_tuple(X, Y, o.R, o.F);
        curX += o.w;
        shelfH = max(shelfH, o.h);
    }
    int H = curY + shelfH;
    int side = max(S, H);
    printf("%d %d\n", side, side);
    for(int i = 0; i < n; i++){
        int X = get<0>(place[i]);
        int Y = get<1>(place[i]);
        int R = get<2>(place[i]);
        int F = get<3>(place[i]);
        printf("%d %d %d %d\n", X, Y, R, F);
    }
    return 0;
}
