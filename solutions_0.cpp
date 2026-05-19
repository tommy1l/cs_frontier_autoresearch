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
        sort(orients[i].begin(), orients[i].end(), [](const Orient& a, const Orient& b){
            if (a.h != b.h) return a.h < b.h;
            return a.w < b.w;
        });
    }

    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b){
        int ma = max(orients[a][0].w, orients[a][0].h);
        int mb = max(orients[b][0].w, orients[b][0].h);
        if (ma != mb) return ma > mb;
        return ks[a] > ks[b];
    });

    int finalSide = 0;
    auto tryPack = [&](int S, vector<tuple<int,int,int,int>>& result) -> bool {
        result.assign(n, make_tuple(0,0,0,0));
        int W = (S + 63) / 64 + 1;
        vector<vector<uint64_t>> grid(S, vector<uint64_t>(W, 0));
        vector<int> sky(S, 0);
        vector<int> prefMax(S+1, 0), sufMax(S+1, 0);
        int curMaxW = 0;
        for (int ii : idx) {
            prefMax[0] = 0;
            for (int i = 0; i < S; i++) prefMax[i+1] = max(prefMax[i], sky[i]);
            sufMax[S] = 0;
            for (int i = S-1; i >= 0; i--) sufMax[i] = max(sufMax[i+1], sky[i]);
            int bestNewMax = INT_MAX, bestY = INT_MAX, bestWaste = INT_MAX, bestX = INT_MAX, bestO = -1;
            for (int o = 0; o < (int)orients[ii].size(); o++) {
                auto& orient = orients[ii][o];
                if (orient.w > S || orient.h > S) continue;
                int w = orient.w, h = orient.h;
                int maxX = S - w;
                for (int X = 0; X <= maxX; X++) {
                    int Y = 0;
                    for (int c = 0; c < w; c++) {
                        if (orient.bot[c] == INT_MAX) continue;
                        int cand = sky[X + c] - orient.bot[c];
                        if (cand > Y) Y = cand;
                    }
                    if (Y + h > S) continue;
                    int waste = 0;
                    for (int c = 0; c < w; c++) {
                        if (orient.bot[c] == INT_MAX) continue;
                        waste += Y + orient.bot[c] - sky[X + c];
                    }
                    int otherMax = max(prefMax[X], sufMax[X+w]);
                    int postH = max(Y + h, otherMax);
                    int postW = max(curMaxW, X + w);
                    int newMax = max(postH, postW);
                    if (newMax > bestNewMax) continue;
                    if (newMax == bestNewMax && Y > bestY) continue;
                    if (newMax == bestNewMax && Y == bestY && waste > bestWaste) continue;
                    if (newMax == bestNewMax && Y == bestY && waste == bestWaste && X >= bestX) continue;
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
                    bestNewMax = newMax; bestY = Y; bestWaste = waste; bestX = X; bestO = o;
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
            if (bestX + orient.w > curMaxW) curMaxW = bestX + orient.w;
        }
        int curMaxH = 0;
        for (int c = 0; c < S; c++) if (sky[c] > curMaxH) curMaxH = sky[c];
        finalSide = max(curMaxW, curMaxH);
        return true;
    };

    int S = (int)ceil(sqrt((double)T));
    if (S < 1) S = 1;
    vector<tuple<int,int,int,int>> result;
    while (!tryPack(S, result)) S++;

    printf("%d %d\n", finalSide, finalSide);
    for (int i = 0; i < n; i++) {
        auto& r = result[i];
        printf("%d %d %d %d\n", get<0>(r), get<1>(r), get<2>(r), get<3>(r));
    }
    return 0;
}
