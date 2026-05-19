// Polyomino packing into a square. Skyline-based greedy bottom-left fill.
#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int F, R;
    int minx, miny;
    int w, h;
    vector<uint64_t> rowMask;
    vector<int> bot; // bot[c] = min cell_y in column c
    vector<int> top; // top[c] = max cell_y in column c
};

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;

    vector<int> ks(n);
    vector<vector<Orient>> orients(n);
    long long T = 0;

    auto rotcw = [](int& x, int& y) { int nx = y, ny = -x; x = nx; y = ny; };

    for (int i = 0; i < n; i++) {
        scanf("%d", &ks[i]);
        T += ks[i];
        vector<pair<int,int>> orig(ks[i]);
        for (int j = 0; j < ks[i]; j++) scanf("%d %d", &orig[j].first, &orig[j].second);
        set<vector<pair<int,int>>> seen;
        for (int f = 0; f < 2; f++) {
            for (int r = 0; r < 4; r++) {
                vector<pair<int,int>> t(ks[i]);
                int minx = INT_MAX, miny = INT_MAX;
                for (int j = 0; j < ks[i]; j++) {
                    int x = orig[j].first, y = orig[j].second;
                    if (f) x = -x;
                    for (int rr = 0; rr < r; rr++) rotcw(x, y);
                    t[j] = {x, y};
                    minx = min(minx, x);
                    miny = min(miny, y);
                }
                for (auto& p : t) { p.first -= minx; p.second -= miny; }
                vector<pair<int,int>> sorted_t = t;
                sort(sorted_t.begin(), sorted_t.end());
                if (!seen.insert(sorted_t).second) continue;
                Orient o;
                o.F = f; o.R = r;
                o.minx = minx; o.miny = miny;
                int mx = 0, my = 0;
                for (auto& p : t) { mx = max(mx, p.first); my = max(my, p.second); }
                o.w = mx + 1; o.h = my + 1;
                o.rowMask.assign(o.h, 0);
                for (auto& p : t) o.rowMask[p.second] |= (1ULL << p.first);
                o.bot.assign(o.w, INT_MAX);
                o.top.assign(o.w, -1);
                for (auto& p : t) {
                    int c = p.first, ry = p.second;
                    if (ry < o.bot[c]) o.bot[c] = ry;
                    if (ry > o.top[c]) o.top[c] = ry;
                }
                orients[i].push_back(o);
            }
        }
    }

    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b){
        int aa = orients[a][0].w * orients[a][0].h;
        int ab = orients[b][0].w * orients[b][0].h;
        if (aa != ab) return aa > ab;
        int ma = max(orients[a][0].w, orients[a][0].h);
        int mb = max(orients[b][0].w, orients[b][0].h);
        if (ma != mb) return ma > mb;
        return ks[a] > ks[b];
    });

    auto tryPack = [&](int S, vector<tuple<int,int,int,int>>& result) -> bool {
        result.assign(n, make_tuple(0,0,0,0));
        int W = (S + 63) / 64 + 1;
        vector<vector<uint64_t>> grid(S, vector<uint64_t>(W, 0));
        vector<int> sky(S, 0);
        for (int ii : idx) {
            int bestY = INT_MAX, bestX = INT_MAX, bestO = -1;
            for (int o = 0; o < (int)orients[ii].size(); o++) {
                auto& orient = orients[ii][o];
                if (orient.w > S || orient.h > S) continue;
                int w = orient.w, h = orient.h;
                int maxX = S - w;
                for (int X = 0; X <= maxX; X++) {
                    int Y = 0;
                    for (int c = 0; c < w; c++) {
                        int cand = sky[X + c] - orient.bot[c];
                        if (cand > Y) Y = cand;
                    }
                    if (Y + h > S) continue;
                    if (Y > bestY || (Y == bestY && X >= bestX)) continue;
                    int wi = X >> 6, bo = X & 63;
                    bool ok = true;
                    for (int r = 0; r < h; r++) {
                        uint64_t m = orient.rowMask[r];
                        if ((m << bo) & grid[Y + r][wi]) { ok = false; break; }
                        if (bo > 0) {
                            if ((m >> (64 - bo)) & grid[Y + r][wi + 1]) { ok = false; break; }
                        }
                    }
                    if (!ok) continue;
                    bestY = Y; bestX = X; bestO = o;
                }
            }
            if (bestO == -1) return false;
            auto& orient = orients[ii][bestO];
            int wi = bestX >> 6, bo = bestX & 63;
            for (int r = 0; r < orient.h; r++) {
                uint64_t m = orient.rowMask[r];
                grid[bestY + r][wi] |= (m << bo);
                if (bo > 0) grid[bestY + r][wi + 1] |= (m >> (64 - bo));
            }
            for (int c = 0; c < orient.w; c++) {
                int newSky = bestY + orient.top[c] + 1;
                if (newSky > sky[bestX + c]) sky[bestX + c] = newSky;
            }
            int Xi = bestX - orient.minx;
            int Yi = bestY - orient.miny;
            result[ii] = make_tuple(Xi, Yi, orient.R, orient.F);
        }
        return true;
    };

    int S = (int)ceil(sqrt((double)T));
    if (S < 1) S = 1;
    vector<tuple<int,int,int,int>> result;
    while (!tryPack(S, result)) S++;

    printf("%d %d\n", S, S);
    for (int i = 0; i < n; i++) {
        auto& r = result[i];
        printf("%d %d %d %d\n", get<0>(r), get<1>(r), get<2>(r), get<3>(r));
    }
    return 0;
}
