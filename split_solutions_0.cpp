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
        for (int j = 0; j < k; j++) {
            scanf("%d %d", &pieces[i][j].first, &pieces[i][j].second);
        }
        total += k;
    }

    struct Choice { int R, F; int w, h; int minx, miny; };
    vector<Choice> choices(n);

    for (int i = 0; i < n; i++) {
        int bestH = INT_MAX, bestW = INT_MAX, bestR = 0, bestF = 0, bestMinX = 0, bestMinY = 0;
        for (int F = 0; F < 2; F++) {
            for (int R = 0; R < 4; R++) {
                int minx = INT_MAX, maxx = INT_MIN, miny = INT_MAX, maxy = INT_MIN;
                for (auto& p : pieces[i]) {
                    int tx = p.first, ty = p.second;
                    if (F) tx = -tx;
                    for (int r = 0; r < R; r++) { int nx = ty; int ny = -tx; tx = nx; ty = ny; }
                    if (tx < minx) minx = tx;
                    if (tx > maxx) maxx = tx;
                    if (ty < miny) miny = ty;
                    if (ty > maxy) maxy = ty;
                }
                int w = maxx - minx + 1;
                int h = maxy - miny + 1;
                if (h < bestH || (h == bestH && w < bestW)) {
                    bestH = h; bestW = w; bestR = R; bestF = F; bestMinX = minx; bestMinY = miny;
                }
            }
        }
        choices[i] = {bestR, bestF, bestW, bestH, bestMinX, bestMinY};
    }

    // Sort pieces by descending height (then descending width) for shelf packing.
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (choices[a].h != choices[b].h) return choices[a].h > choices[b].h;
        return choices[a].w > choices[b].w;
    });

    int bestSide = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacement;

    // Try several W values, pick the one that minimizes the bounding square side.
    double sqrtT = sqrt((double)total);
    for (double f = 1.0; f <= 1.5 + 1e-9; f += 0.05) {
        int W = max(10, (int)ceil(sqrtT * f));

        vector<tuple<int,int,int,int>> placement(n);
        int shelfY = 0, shelfH = 0, shelfX = 0;
        int maxH = 0;
        bool ok = true;

        for (int idx : order) {
            const Choice& c = choices[idx];
            if (c.w > W) { ok = false; break; }
            if (shelfX + c.w > W) {
                shelfY += shelfH;
                shelfH = 0;
                shelfX = 0;
            }
            int X = shelfX - c.minx;
            int Y = shelfY - c.miny;
            placement[idx] = make_tuple(X, Y, c.R, c.F);
            shelfX += c.w;
            if (c.h > shelfH) shelfH = c.h;
            int curTop = shelfY + shelfH;
            if (curTop > maxH) maxH = curTop;
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
