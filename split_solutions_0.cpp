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

    struct Variant {
        int R, F;
        int w, h;
        int minx, miny;
        vector<pair<int,int>> cells; // normalized [0,w) x [0,h)
        vector<pair<int,int>> bottomCols; // (cx, min cellY at cx)
        int leftTopY;  // max cellY among cells with cx == 0
        int rightTopY; // max cellY among cells with cx == w-1
        int topRough;  // max - min of per-column max cellY
    };
    vector<vector<Variant>> variants(n);
    vector<int> maxDim(n, 0), kSize(n, 0);

    for (int i = 0; i < n; i++) {
        set<vector<pair<int,int>>> seen;
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
                vector<pair<int,int>> norm;
                norm.reserve(tcells.size());
                for (auto& c : tcells) norm.push_back({c.first - minx, c.second - miny});
                sort(norm.begin(), norm.end());
                if (seen.count(norm)) continue;
                seen.insert(norm);
                Variant v;
                v.R = R; v.F = F;
                v.w = w; v.h = h;
                v.minx = minx; v.miny = miny;
                v.cells = norm;
                // norm is sorted by (x asc, y asc), so first per-x is the bottom cell.
                int curX = -1;
                int leftTopY = -1, rightTopY = -1;
                int curColMaxY = -1;
                int topYMin = INT_MAX, topYMax = INT_MIN;
                for (auto& cell : norm) {
                    if (cell.first != curX) {
                        if (curX != -1) {
                            if (curColMaxY < topYMin) topYMin = curColMaxY;
                            if (curColMaxY > topYMax) topYMax = curColMaxY;
                        }
                        v.bottomCols.push_back(cell);
                        curX = cell.first;
                        curColMaxY = cell.second;
                    } else {
                        curColMaxY = cell.second;
                    }
                    if (cell.first == 0) leftTopY = cell.second;
                    if (cell.first == v.w - 1) rightTopY = cell.second;
                }
                if (curX != -1) {
                    if (curColMaxY < topYMin) topYMin = curColMaxY;
                    if (curColMaxY > topYMax) topYMax = curColMaxY;
                }
                v.leftTopY = leftTopY;
                v.rightTopY = rightTopY;
                v.topRough = (topYMax >= topYMin) ? (topYMax - topYMin) : 0;
                variants[i].push_back(v);
            }
        }
        maxDim[i] = max(variants[i][0].w, variants[i][0].h);
        kSize[i] = (int)pieces[i].size();
    }

    // Rotation-invariant sort: largest max dimension first, tiebreak by k.
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (maxDim[a] != maxDim[b]) return maxDim[a] > maxDim[b];
        return kSize[a] > kSize[b];
    });

    int bestSide = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacement;

    double sqrtT = sqrt((double)total);
    // Per-placement orientation is 5-8x heavier per X; keep sweep narrow.
    for (double f = 1.00; f <= 1.15 + 1e-9; f += 0.05) {
        int W = max(10, (int)ceil(sqrtT * f));
        vector<int> col(W, 0);
        vector<tuple<int,int,int,int>> placement(n);
        bool ok = true;
        int maxH = 0;

        for (int idx : order) {
            int bestMetric = INT_MAX;
            int bestY = -1, bestX = -1, bestV = -1;
            const auto& vs = variants[idx];
            for (int vi = 0; vi < (int)vs.size(); vi++) {
                const Variant& c = vs[vi];
                if (c.w > W) continue;
                int xMax = W - c.w;
                for (int X = 0; X <= xMax; X++) {
                    int yMin = 0;
                    for (auto& cell : c.cells) {
                        int need = col[X + cell.first] - cell.second;
                        if (need > yMin) yMin = need;
                    }
                    int waste = 0;
                    for (auto& bc : c.bottomCols) {
                        waste += (yMin + bc.second) - col[X + bc.first];
                    }
                    int top = yMin + c.h;
                    int leftPieceTop = yMin + c.leftTopY + 1;
                    int rightPieceTop = yMin + c.rightTopY + 1;
                    int leftExposed = (X > 0) ? max(0, leftPieceTop - col[X - 1]) : 0;
                    int rightExposed = (X + c.w < W) ? max(0, rightPieceTop - col[X + c.w]) : 0;
                    int metric = top + waste + leftExposed + rightExposed + c.topRough;
                    if (metric < bestMetric) {
                        bestMetric = metric;
                        bestY = yMin;
                        bestX = X;
                        bestV = vi;
                    }
                }
            }
            if (bestV < 0) { ok = false; break; }
            const Variant& c = vs[bestV];
            int outX = bestX - c.minx;
            int outY = bestY - c.miny;
            placement[idx] = make_tuple(outX, outY, c.R, c.F);
            for (auto& cell : c.cells) {
                int gx = bestX + cell.first;
                int top = bestY + cell.second + 1;
                if (top > col[gx]) col[gx] = top;
                if (top > maxH) maxH = top;
            }
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
