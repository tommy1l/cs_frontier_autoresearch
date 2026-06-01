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
    for (int i = 0; i < n; i++) {
        for (auto& o : orientations[i]) {
            maxDim[i] = max(maxDim[i], max(o.width, o.height));
        }
    }
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (maxDim[a] != maxDim[b]) return maxDim[a] > maxDim[b];
        return polys[a].size() > polys[b].size();
    });

    int L = max(1, (int)ceil(sqrt((double)totalArea * 1.10)));

    vector<tuple<int,int,int,int>> placements(n);

    auto runGreedy = [&](int L, vector<tuple<int,int,int,int>>& outPlacements, int& outMaxX, int& outMaxY) -> bool {
        vector<int> skyline(L, 0);
        outPlacements.assign(n, make_tuple(0, 0, 0, 0));
        outMaxX = -1;
        outMaxY = -1;
        for (int idx : order) {
            int curMax = 0;
            for (int v : skyline) if (v > curMax) curMax = v;
            int bestNewMax = INT_MAX, bestGap = INT_MAX, bestTop = INT_MAX, bestY = INT_MAX, bestX = INT_MAX, bestO = -1;
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
                    int totalGap = 0;
                    for (int dx = 0; dx < o.width; dx++) {
                        if (o.minDy[dx] == INT_MAX) continue;
                        totalGap += (Y + o.minDy[dx]) - skyline[X + dx];
                    }
                    int top = Y + (o.height - 1);
                    int newMax = max(curMax, top + 1);
                    bool better = false;
                    if (newMax < bestNewMax) better = true;
                    else if (newMax == bestNewMax) {
                        if (totalGap < bestGap) better = true;
                        else if (totalGap == bestGap) {
                            if (top < bestTop) better = true;
                            else if (top == bestTop) {
                                if (Y < bestY) better = true;
                                else if (Y == bestY) {
                                    if (X < bestX) better = true;
                                    else if (X == bestX && oi < bestO) better = true;
                                }
                            }
                        }
                    }
                    if (better) {
                        bestNewMax = newMax;
                        bestGap = totalGap;
                        bestTop = top;
                        bestY = Y;
                        bestX = X;
                        bestO = oi;
                    }
                }
            }
            if (bestO == -1) return false;
            auto& o = orientations[idx][bestO];
            for (int dx = 0; dx < o.width; dx++) {
                if (o.maxDy[dx] == INT_MIN) continue;
                int nh = bestY + o.maxDy[dx] + 1;
                if (nh > skyline[bestX + dx]) skyline[bestX + dx] = nh;
            }
            for (auto& c : o.cells) {
                int wx = bestX + c.first;
                int wy = bestY + c.second;
                if (wx > outMaxX) outMaxX = wx;
                if (wy > outMaxY) outMaxY = wy;
            }
            int Tx = bestX - o.shiftX;
            int Ty = bestY - o.shiftY;
            outPlacements[idx] = make_tuple(Tx, Ty, o.R, o.F);
        }
        return true;
    };

    int curMaxX = -1, curMaxY = -1;
    while (true) {
        vector<tuple<int,int,int,int>> tryP;
        int mx, my;
        if (runGreedy(L, tryP, mx, my)) {
            placements = std::move(tryP);
            curMaxX = mx;
            curMaxY = my;
            break;
        }
        L++;
    }

    int shrinkAttempts = 0;
    while (shrinkAttempts < 4) {
        int Lnew = max(curMaxX + 1, curMaxY + 1);
        if (Lnew >= L) break;
        vector<tuple<int,int,int,int>> tryP;
        int mx, my;
        if (runGreedy(Lnew, tryP, mx, my)) {
            placements = std::move(tryP);
            L = Lnew;
            curMaxX = mx;
            curMaxY = my;
            shrinkAttempts++;
        } else {
            break;
        }
    }

    int outL = max(curMaxX + 1, curMaxY + 1);
    cout << outL << " " << outL << "\n";
    for (int i = 0; i < n; i++) {
        int X, Y, R, F;
        tie(X, Y, R, F) = placements[i];
        cout << X << " " << Y << " " << R << " " << F << "\n";
    }

    return 0;
}
