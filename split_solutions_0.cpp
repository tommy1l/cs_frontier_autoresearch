#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;

    vector<vector<pair<int,int>>> pieces(n);
    long long total = 0;
    for (int i = 0; i < n; i++) {
        int k;
        scanf("%d", &k);
        pieces[i].resize(k);
        for (int j = 0; j < k; j++) scanf("%d %d", &pieces[i][j].first, &pieces[i][j].second);
        total += k;
    }

    struct Choice {
        int R, F;
        int w, h;
        int minx, miny;
        vector<pair<int,int>> cells; // normalized to [0,w)x[0,h)
    };
    vector<Choice> choices(n);

    for (int i = 0; i < n; i++) {
        int bestH = INT_MAX, bestW = INT_MAX, bestR = 0, bestF = 0;
        int bestMinX = 0, bestMinY = 0;
        vector<pair<int,int>> bestCells;
        for (int F = 0; F < 2; F++) {
            for (int R = 0; R < 4; R++) {
                int minx = INT_MAX, maxx = INT_MIN, miny = INT_MAX, maxy = INT_MIN;
                vector<pair<int,int>> tcells;
                tcells.reserve(pieces[i].size());
                for (auto& p : pieces[i]) {
                    int tx = p.first, ty = p.second;
                    if (F) tx = -tx;
                    for (int r = 0; r < R; r++) { int nx = ty; int ny = -tx; tx = nx; ty = ny; }
                    tcells.push_back({tx, ty});
                    if (tx < minx) minx = tx;
                    if (tx > maxx) maxx = tx;
                    if (ty < miny) miny = ty;
                    if (ty > maxy) maxy = ty;
                }
                int w = maxx - minx + 1;
                int h = maxy - miny + 1;
                if (h < bestH || (h == bestH && w < bestW)) {
                    bestH = h; bestW = w; bestR = R; bestF = F;
                    bestMinX = minx; bestMinY = miny;
                    bestCells.clear();
                    for (auto& c : tcells) bestCells.push_back({c.first - minx, c.second - miny});
                }
            }
        }
        choices[i].R = bestR;
        choices[i].F = bestF;
        choices[i].w = bestW;
        choices[i].h = bestH;
        choices[i].minx = bestMinX;
        choices[i].miny = bestMinY;
        choices[i].cells = bestCells;
    }

    // Sort by descending h, then descending w (classic BFDH for BLF).
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (choices[a].h != choices[b].h) return choices[a].h > choices[b].h;
        return choices[a].w > choices[b].w;
    });

    int bestSide = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacement;

    double sqrtT = sqrt((double)total);
    // BLF packs much tighter than shelf; sweep a narrow band around sqrt(T).
    for (double f = 1.0; f <= 1.25 + 1e-9; f += 0.05) {
        int W = max(10, (int)ceil(sqrtT * f));
        vector<int> col(W, 0);                     // skyline: 1 + max y of occupied cell in column
        vector<tuple<int,int,int,int>> placement(n);
        bool ok = true;
        int maxH = 0;

        for (int idx : order) {
            const Choice& c = choices[idx];
            if (c.w > W) { ok = false; break; }
            int bestY = INT_MAX, bestX = -1;
            int xMax = W - c.w;
            for (int X = 0; X <= xMax; X++) {
                int yMin = 0;
                for (auto& cell : c.cells) {
                    int need = col[X + cell.first] - cell.second;
                    if (need > yMin) yMin = need;
                }
                if (yMin < bestY) { bestY = yMin; bestX = X; }
            }
            int outX = bestX - c.minx;
            int outY = bestY - c.miny;
            placement[idx] = make_tuple(outX, outY, c.R, c.F);
            for (auto& cell : c.cells) {
                int gx = bestX + cell.first;
                int top = bestY + cell.second + 1;
                if (top > col[gx]) col[gx] = top;
                if (top > maxH) maxH = top;
            }
            // Prune: if already worse than best known square side.
            if (bestSide != INT_MAX && maxH >= bestSide) { ok = false; break; }
        }
        if (!ok) continue;
        int side = max(W, maxH);
        if (side < bestSide) {
            bestSide = side;
            bestPlacement = placement;
        }
    }

    printf("%d %d\n", bestSide, bestSide);
    for (int i = 0; i < n; i++) {
        int X, Y, R, F;
        tie(X, Y, R, F) = bestPlacement[i];
        printf("%d %d %d %d\n", X, Y, R, F);
    }
    return 0;
}
