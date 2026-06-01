#include <bits/stdc++.h>
using namespace std;

struct Orientation {
    int R, F;
    int width, height;
    int shiftX, shiftY;
    vector<int> minDy, maxDy;
    vector<pair<int,int>> cells;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<vector<pair<int,int>>> polys(n);
    long long totalArea = 0;
    for (int i = 0; i < n; i++) {
        int k;
        cin >> k;
        polys[i].resize(k);
        for (int j = 0; j < k; j++) {
            cin >> polys[i][j].first >> polys[i][j].second;
        }
        totalArea += k;
    }

    vector<vector<Orientation>> orientations(n);
    for (int i = 0; i < n; i++) {
        set<vector<pair<int,int>>> seen;
        for (int F = 0; F < 2; F++) {
            for (int R = 0; R < 4; R++) {
                vector<pair<int,int>> cells;
                cells.reserve(polys[i].size());
                for (auto& c : polys[i]) {
                    int x = c.first, y = c.second;
                    if (F) x = -x;
                    for (int r = 0; r < R; r++) {
                        int nx = y, ny = -x;
                        x = nx; y = ny;
                    }
                    cells.push_back({x, y});
                }
                int minX = INT_MAX, minY = INT_MAX;
                for (auto& c : cells) {
                    minX = min(minX, c.first);
                    minY = min(minY, c.second);
                }
                int sx = minX, sy = minY;
                for (auto& c : cells) {
                    c.first -= sx;
                    c.second -= sy;
                }
                vector<pair<int,int>> sorted_cells = cells;
                sort(sorted_cells.begin(), sorted_cells.end());
                if (seen.count(sorted_cells)) continue;
                seen.insert(sorted_cells);

                Orientation o;
                o.R = R;
                o.F = F;
                o.shiftX = sx;
                o.shiftY = sy;
                o.cells = cells;
                int maxX = 0, maxY = 0;
                for (auto& c : cells) {
                    maxX = max(maxX, c.first);
                    maxY = max(maxY, c.second);
                }
                o.width = maxX + 1;
                o.height = maxY + 1;
                o.minDy.assign(o.width, INT_MAX);
                o.maxDy.assign(o.width, INT_MIN);
                for (auto& c : cells) {
                    if (c.second < o.minDy[c.first]) o.minDy[c.first] = c.second;
                    if (c.second > o.maxDy[c.first]) o.maxDy[c.first] = c.second;
                }
                orientations[i].push_back(o);
            }
        }
    }

    vector<int> maxDim(n, 0);
    vector<int> minDim(n, 0);
    for (int i = 0; i < n; i++) {
        int mx = 0, mn = INT_MAX;
        for (auto& o : orientations[i]) {
            mx = max(mx, max(o.width, o.height));
        }
        // bbox of orientation 0
        auto& o0 = orientations[i][0];
        int w0 = o0.width, h0 = o0.height;
        maxDim[i] = mx;
        minDim[i] = min(w0, h0);
    }

    int initL = max(1, (int)ceil(sqrt((double)totalArea * 1.10)));

    auto runPack = [&](const vector<int>& order, int& outL, vector<tuple<int,int,int,int>>& outPlacements) {
        int L = initL;
        vector<tuple<int,int,int,int>> placements(n);
        while (true) {
            vector<int> skyline(L, 0);
            bool failed = false;
            for (int idx : order) {
                int bestY = INT_MAX, bestX = INT_MAX, bestO = -1, bestSupport = -1;
                int numO = (int)orientations[idx].size();
                for (int oi = 0; oi < numO; oi++) {
                    auto& o = orientations[idx][oi];
                    if (o.width > L || o.height > L) continue;
                    int wlim = L - o.width;
                    for (int X = 0; X <= wlim; X++) {
                        int Y = 0;
                        for (int dx = 0; dx < o.width; dx++) {
                            if (o.minDy[dx] == INT_MAX) continue;
                            int need = skyline[X + dx] - o.minDy[dx];
                            if (need > Y) Y = need;
                        }
                        bool fits = true;
                        for (int dx = 0; dx < o.width; dx++) {
                            if (o.maxDy[dx] == INT_MIN) continue;
                            if (Y + o.maxDy[dx] >= L) { fits = false; break; }
                        }
                        if (!fits) continue;
                        int support = 0;
                        for (int dx = 0; dx < o.width; dx++) {
                            if (o.minDy[dx] == INT_MAX) continue;
                            if (Y + o.minDy[dx] == skyline[X + dx]) support++;
                        }
                        bool better = false;
                        if (Y < bestY) better = true;
                        else if (Y == bestY) {
                            if (support > bestSupport) better = true;
                            else if (support == bestSupport) {
                                if (X < bestX) better = true;
                                else if (X == bestX && oi < bestO) better = true;
                            }
                        }
                        if (better) {
                            bestY = Y; bestX = X; bestO = oi; bestSupport = support;
                        }
                    }
                }
                if (bestO == -1) { failed = true; break; }
                auto& o = orientations[idx][bestO];
                for (int dx = 0; dx < o.width; dx++) {
                    if (o.maxDy[dx] == INT_MIN) continue;
                    int nh = bestY + o.maxDy[dx] + 1;
                    if (nh > skyline[bestX + dx]) skyline[bestX + dx] = nh;
                }
                int Tx = bestX - o.shiftX;
                int Ty = bestY - o.shiftY;
                placements[idx] = make_tuple(Tx, Ty, o.R, o.F);
            }
            if (!failed) break;
            L++;
        }
        outL = L;
        outPlacements = std::move(placements);
    };

    vector<int> orderA(n), orderB(n), orderC(n);
    iota(orderA.begin(), orderA.end(), 0);
    iota(orderB.begin(), orderB.end(), 0);
    iota(orderC.begin(), orderC.end(), 0);
    sort(orderA.begin(), orderA.end(), [&](int a, int b) {
        if (maxDim[a] != maxDim[b]) return maxDim[a] > maxDim[b];
        return polys[a].size() > polys[b].size();
    });
    sort(orderB.begin(), orderB.end(), [&](int a, int b) {
        if (polys[a].size() != polys[b].size()) return polys[a].size() > polys[b].size();
        return maxDim[a] > maxDim[b];
    });
    sort(orderC.begin(), orderC.end(), [&](int a, int b) {
        int areaA = maxDim[a] * minDim[a];
        int areaB = maxDim[b] * minDim[b];
        if (areaA != areaB) return areaA > areaB;
        return polys[a].size() > polys[b].size();
    });

    int bestL = INT_MAX;
    vector<tuple<int,int,int,int>> bestPlacements;
    for (auto& ord : {orderA, orderB, orderC}) {
        int L;
        vector<tuple<int,int,int,int>> pl;
        runPack(ord, L, pl);
        if (L < bestL) {
            bestL = L;
            bestPlacements = std::move(pl);
        }
    }

    cout << bestL << " " << bestL << "\n";
    for (int i = 0; i < n; i++) {
        int X, Y, R, F;
        tie(X, Y, R, F) = bestPlacements[i];
        cout << X << " " << Y << " " << R << " " << F << "\n";
    }

    return 0;
}
