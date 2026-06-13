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
                int curX = -1;
                for (auto& cell : norm) {
                    if (cell.first != curX) {
                        v.bottomCols.push_back(cell);
                        curX = cell.first;
                    }
                }
                variants[i].push_back(v);
            }
        }
        maxDim[i] = max(variants[i][0].w, variants[i][0].h);
        kSize[i] = (int)pieces[i].size();
    }

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (maxDim[a] != maxDim[b]) return maxDim[a] > maxDim[b];
        return kSize[a] > kSize[b];
    });

    int bestSide = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacement;

    double sqrtT = sqrt((double)total);
    for (double f = 1.00; f <= 1.15 + 1e-9; f += 0.05) {
        int W = max(10, (int)ceil(sqrtT * f));
        vector<int> col(W, 0);
        vector<tuple<int,int,int>> internalPlace(n); // (X, Y, vi)
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
                    int metric = top + waste;
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
            internalPlace[idx] = make_tuple(bestX, bestY, bestV);
            for (auto& cell : c.cells) {
                int gx = bestX + cell.first;
                int top = bestY + cell.second + 1;
                if (top > col[gx]) col[gx] = top;
                if (top > maxH) maxH = top;
            }
            if (bestSide != INT_MAX && maxH >= bestSide) { ok = false; break; }
        }
        if (!ok) continue;

        // Post-pass: gravity drop. Lets pieces fall into pockets/holes
        // inside concave shapes that the column-skyline BLF cannot exploit.
        int H = maxH;
        vector<vector<int>> occ(W, vector<int>(H + 1, 0));
        for (int i = 0; i < n; i++) {
            int X = get<0>(internalPlace[i]);
            int Y = get<1>(internalPlace[i]);
            int vi = get<2>(internalPlace[i]);
            const auto& cells = variants[i][vi].cells;
            for (auto& cc : cells) occ[X + cc.first][Y + cc.second] = i + 1;
        }
        vector<int> dropOrd(n);
        iota(dropOrd.begin(), dropOrd.end(), 0);
        sort(dropOrd.begin(), dropOrd.end(), [&](int a, int b) {
            return get<1>(internalPlace[a]) < get<1>(internalPlace[b]);
        });
        for (int it = 0; it < 2; it++) {
            bool moved = false;
            for (int p : dropOrd) {
                int X = get<0>(internalPlace[p]);
                int Y = get<1>(internalPlace[p]);
                int vi = get<2>(internalPlace[p]);
                const auto& cells = variants[p][vi].cells;
                for (auto& cc : cells) occ[X + cc.first][Y + cc.second] = 0;
                int newY = Y;
                for (int yp = 0; yp <= Y; yp++) {
                    bool good = true;
                    for (auto& cc : cells) {
                        if (occ[X + cc.first][yp + cc.second]) { good = false; break; }
                    }
                    if (good) { newY = yp; break; }
                }
                for (auto& cc : cells) occ[X + cc.first][newY + cc.second] = p + 1;
                if (newY < Y) { internalPlace[p] = make_tuple(X, newY, vi); moved = true; }
            }
            if (!moved) break;
            sort(dropOrd.begin(), dropOrd.end(), [&](int a, int b) {
                return get<1>(internalPlace[a]) < get<1>(internalPlace[b]);
            });
        }
        int newMaxH = 0;
        for (int i = 0; i < n; i++) {
            int Y = get<1>(internalPlace[i]);
            int vi = get<2>(internalPlace[i]);
            int top = Y + variants[i][vi].h;
            if (top > newMaxH) newMaxH = top;
        }

        for (int i = 0; i < n; i++) {
            int X = get<0>(internalPlace[i]);
            int Y = get<1>(internalPlace[i]);
            int vi = get<2>(internalPlace[i]);
            const Variant& v = variants[i][vi];
            placement[i] = make_tuple(X - v.minx, Y - v.miny, v.R, v.F);
        }

        int side = max(W, newMaxH);
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
