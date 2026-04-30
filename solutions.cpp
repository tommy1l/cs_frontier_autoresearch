/**
 * Polyomino Packing Solver
 * ========================
 * Strategy:
 *   1. Precompute all ≤8 distinct orientations (rotate × reflect) for each piece,
 *      normalized so min-x=0, min-y=0. Record the normalization offset (minTx, minTy)
 *      so the output translation can be corrected.
 *   2. Linear scan on side length S starting from ceil(sqrt(total_cells)).
 *   3. Greedy packer with a column-level skyline:
 *        - Sort pieces largest-first (by cell count).
 *        - For each piece, scan every column cx from left to right; for each orientation
 *          compute the minimum valid cy (max skyline under the piece's footprint), then
 *          scan upward until no grid conflict.
 *        - Among all (orientation, cx, cy) combos choose the one that minimises cy
 *          (bottommost placement), breaking ties by cx (leftmost).
 *   4. Output: translation X = bestCX - minTx, Y = bestCY - minTy (corrects for normalization).
 *
 * Transform convention (spec, applied in order):
 *   1. Reflect (F=1): x → −x
 *   2. Rotate R times 90° clockwise: (x,y) → (y,−x)
 *   3. Translate by (X,Y)
 *
 * Verification identity: placed_cell = transform(raw_cell, R, F) + (X, Y)
 */

#include <bits/stdc++.h>
using namespace std;

static inline pair<int, int> applyTransform(int x, int y, int r, int f)
{
    if (f)
        x = -x;
    for (int i = 0; i < r; i++)
    {
        int nx = y, ny = -x;
        x = nx;
        y = ny;
    }
    return {x, y};
}

struct Piece
{
    vector<pair<int, int>> cells; // normalized: shifted so min-x=0, min-y=0
    int W, H;                     // bounding box
    int r, f;                     // transform params
    int minTx, minTy;             // normalization offset (min of transformed coords before shift)
    // Output translation: X = bestCX - minTx, Y = bestCY - minTy
};

static Piece makePiece(const vector<pair<int, int>> &raw, int r, int f)
{
    Piece p;
    p.r = r;
    p.f = f;
    int minx = INT_MAX, miny = INT_MAX;
    vector<pair<int, int>> transformed;
    for (auto [x, y] : raw)
    {
        auto [tx, ty] = applyTransform(x, y, r, f);
        minx = min(minx, tx);
        miny = min(miny, ty);
        transformed.push_back({tx, ty});
    }
    p.minTx = minx;
    p.minTy = miny;
    for (auto [tx, ty] : transformed)
        p.cells.push_back({tx - minx, ty - miny});
    int maxx = 0, maxy = 0;
    for (auto [x, y] : p.cells)
    {
        maxx = max(maxx, x);
        maxy = max(maxy, y);
    }
    p.W = maxx + 1;
    p.H = maxy + 1;
    return p;
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<pair<int, int>>> raw(n);
    long long totalCells = 0;
    for (int i = 0; i < n; i++)
    {
        int k;
        cin >> k;
        raw[i].resize(k);
        for (int j = 0; j < k; j++)
            cin >> raw[i][j].first >> raw[i][j].second;
        totalCells += k;
    }

    // Precompute distinct orientations for each piece
    vector<vector<Piece>> orientations(n);
    for (int i = 0; i < n; i++)
    {
        set<vector<pair<int, int>>> seen;
        for (int f = 0; f < 2; f++)
        {
            for (int r = 0; r < 4; r++)
            {
                Piece p = makePiece(raw[i], r, f);
                auto cells = p.cells;
                sort(cells.begin(), cells.end());
                if (seen.insert(cells).second)
                    orientations[i].push_back(p);
            }
        }
    }

    // Sort pieces: largest cell count first
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b)
         { return (int)raw[a].size() > (int)raw[b].size(); });

    int Smin = (int)ceil(sqrt((double)totalCells));

    struct Placement
    {
        int X, Y, R, F;
    };

    auto tryPack = [&](int S) -> pair<bool, vector<Placement>>
    {
        vector<bool> grid((size_t)S * S, false);
        vector<int> skyline(S, 0); // skyline[x] = next free y in column x

        vector<Placement> placements(n);

        for (int idx : order)
        {
            const auto &orients = orientations[idx];
            int numO = (int)orients.size();

            int bestScore = INT_MAX; // cy*S + cx, lower = better
            int bestCX = -1, bestCY = -1, bestOI = -1;

            for (int oi = 0; oi < numO; oi++)
            {
                const Piece &p = orients[oi];
                if (p.W > S || p.H > S)
                    continue;

                for (int cx = 0; cx <= S - p.W; cx++)
                {
                    // Lower bound on cy from skyline: for each cell (dx,dy), need cy+dy >= skyline[cx+dx]
                    int minCY = 0;
                    for (auto [dx, dy] : p.cells)
                        minCY = max(minCY, skyline[cx + dx] - dy);
                    if (minCY < 0)
                        minCY = 0;
                    if (minCY + p.H > S)
                        continue;
                    if (minCY * S + cx >= bestScore)
                        continue; // can't beat current best

                    // Scan upward from minCY until valid
                    for (int cy = minCY; cy + p.H <= S; cy++)
                    {
                        bool ok = true;
                        for (auto [dx, dy] : p.cells)
                        {
                            if (grid[(cy + dy) * S + (cx + dx)])
                            {
                                ok = false;
                                break;
                            }
                        }
                        if (ok)
                        {
                            int score = cy * S + cx;
                            if (score < bestScore)
                            {
                                bestScore = score;
                                bestCX = cx;
                                bestCY = cy;
                                bestOI = oi;
                            }
                            break;
                        }
                    }
                }
            }

            if (bestOI < 0)
                return {false, {}};

            // Commit placement
            const Piece &p = orients[bestOI];
            for (auto [dx, dy] : p.cells)
            {
                int gx = bestCX + dx, gy = bestCY + dy;
                grid[gy * S + gx] = true;
                skyline[gx] = max(skyline[gx], gy + 1);
            }

            // Correct translation for normalization offset
            placements[idx] = {bestCX - p.minTx, bestCY - p.minTy, p.r, p.f};
        }

        return {true, placements};
    };

    auto startTime = chrono::steady_clock::now();
    auto elapsed_ms = [&]() -> long long {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - startTime).count();
    };

    int bestS = -1;
    vector<Placement> bestPlacements;

    // Phase 1: find initial solution with sorted order
    for (int S = Smin; bestS < 0 && S <= Smin + 500; S++)
    {
        auto [ok, placements] = tryPack(S);
        if (ok)
        {
            bestS = S;
            bestPlacements = placements;
        }
    }

    if (bestS >= 0)
    {
        // Phase 2: try smaller S with random piece orderings within time budget
        mt19937 rng(42);
        int targetS = bestS - 1;
        int attemptsAtTarget = 0;

        while (targetS >= Smin && elapsed_ms() < 8000 && attemptsAtTarget < 50)
        {
            shuffle(order.begin(), order.end(), rng);
            auto [ok, placements] = tryPack(targetS);
            if (ok)
            {
                bestS = targetS;
                bestPlacements = placements;
                targetS--;
                attemptsAtTarget = 0;
            }
            else
            {
                attemptsAtTarget++;
            }
        }

        // Shrink square to actual used bounding box
        int maxUsedX = 0, maxUsedY = 0;
        for (int i = 0; i < n; i++)
        {
            auto &pl = bestPlacements[i];
            for (auto &[cx, cy] : raw[i])
            {
                auto [tx, ty] = applyTransform(cx, cy, pl.R, pl.F);
                maxUsedX = max(maxUsedX, tx + pl.X + 1);
                maxUsedY = max(maxUsedY, ty + pl.Y + 1);
            }
        }
        int actualS = max(maxUsedX, maxUsedY);

        cout << actualS << " " << actualS << "\n";
        for (int i = 0; i < n; i++)
        {
            auto &pl = bestPlacements[i];
            cout << pl.X << " " << pl.Y << " " << pl.R << " " << pl.F << "\n";
        }
        return 0;
    }

    // Fallback: stack each piece in its first orientation, one per row
    {
        int maxPieceH = 0;
        for (int i = 0; i < n; i++)
            maxPieceH = max(maxPieceH, orientations[i][0].H);
        long long totalH = 0;
        for (int i = 0; i < n; i++)
            totalH += orientations[i][0].H;
        // Need width >= max piece width, height >= totalH
        int maxPieceW = 0;
        for (int i = 0; i < n; i++)
            maxPieceW = max(maxPieceW, orientations[i][0].W);
        int SF = (int)max((long long)maxPieceW, totalH);
        SF = max(SF, Smin);
        cout << SF << " " << SF << "\n";
        int curY = 0;
        for (int i = 0; i < n; i++)
        {
            const Piece &p = orientations[i][0];
            cout << (0 - p.minTx) << " " << (curY - p.minTy) << " " << p.r << " " << p.f << "\n";
            curY += p.H;
        }
    }
    return 0;
}