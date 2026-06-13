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

    // Split monominoes out of the BLF pass — they fill cavities post-hoc.
    vector<int> order_big, order_mono;
    for (int i : order) {
        if (kSize[i] == 1) order_mono.push_back(i);
        else order_big.push_back(i);
    }

    int bestSide = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacement;

    double sqrtT = sqrt((double)total);
    // Per-placement orientation is 5-8x heavier per X; keep sweep narrow.
    for (double f = 1.00; f <= 1.15 + 1e-9; f += 0.05) {
        int W = max(10, (int)ceil(sqrtT * f));
        vector<int> col(W, 0);
        vector<vector<int>> occ(W); // occupied y values per column (for hole detection)
        vector<tuple<int,int,int,int>> placement(n);
        bool ok = true;
        int maxH = 0;

        for (int idx : order_big) {
            int bestTop = INT_MAX;
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
                    int top = yMin + c.h;
                    if (top < bestTop) {
                        bestTop = top;
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
                int gy = bestY + cell.second;
                int top = gy + 1;
                occ[gx].push_back(gy);
                if (top > col[gx]) col[gx] = top;
                if (top > maxH) maxH = top;
            }
            if (bestSide != INT_MAX && maxH >= bestSide) { ok = false; break; }
        }
        if (!ok) continue;

        // Hole-filling for monominoes: enumerate cavity cells (below skyline, unoccupied),
        // place monominoes there first; remaining monominoes go on the lowest column.
        vector<pair<int,int>> holes; // (x, y) — cavity slots
        for (int x = 0; x < W; x++) {
            sort(occ[x].begin(), occ[x].end());
            int idxOcc = 0;
            for (int y = 0; y < col[x]; y++) {
                if (idxOcc < (int)occ[x].size() && occ[x][idxOcc] == y) {
                    idxOcc++;
                } else {
                    holes.push_back({x, y});
                }
            }
        }
        // Lowest holes first — keeps the upper rectangle clean.
        sort(holes.begin(), holes.end(), [](const pair<int,int>& a, const pair<int,int>& b) {
            if (a.second != b.second) return a.second < b.second;
            return a.first < b.first;
        });

        int hi = 0;
        for (int idx : order_mono) {
            int gx, gy;
            if (hi < (int)holes.size()) {
                gx = holes[hi].first;
                gy = holes[hi].second;
                hi++;
            } else {
                int lo = 0;
                for (int xi = 1; xi < W; xi++) if (col[xi] < col[lo]) lo = xi;
                gx = lo;
                gy = col[lo];
                col[lo]++;
                if (col[lo] > maxH) maxH = col[lo];
                if (bestSide != INT_MAX && maxH >= bestSide) { ok = false; break; }
            }
            const Variant& c = variants[idx][0];
            placement[idx] = make_tuple(gx - c.minx, gy - c.miny, c.R, c.F);
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
