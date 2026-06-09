#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h;
    vector<pair<int,int>> cells;
    vector<int> cols;
    vector<int> min_dy;
    vector<int> max_dy;
    int min_tx, min_ty;
    vector<vector<int>> pattern_dx;
};

int n;
vector<vector<Orient>> orients;
vector<int> ks;
int W;
int NW;
vector<int> priorityOrder;
vector<int> min_w_orient, min_h_orient;

bool tryPack(vector<vector<uint64_t>>& col_bits,
             vector<int>& tX, vector<int>& tY, vector<int>& tR, vector<int>& tF){
    for(auto& cb : col_bits) fill(cb.begin(), cb.end(), 0ULL);
    
    vector<int> skyline(W, 0);
    vector<char> placed(n, 0);
    int remaining = n;
    const int K = 24;
    
    while(remaining > 0){
        int min_h = INT_MAX;
        for(int c=0;c<W;c++) min_h = min(min_h, skyline[c]);
        int lo = 0;
        while(lo < W && skyline[lo] != min_h) lo++;
        int hi = lo+1;
        while(hi < W && skyline[hi] == min_h) hi++;
        int w_gap = hi - lo;
        int h_left = (lo==0) ? INT_MAX : skyline[lo-1];
        int h_right = (hi==W) ? INT_MAX : skyline[hi];
        
        // build candidate pool
        vector<int> pool;
        for(int pi : priorityOrder){
            if(placed[pi]) continue;
            if(min_w_orient[pi] > w_gap) continue;
            if(min_h + min_h_orient[pi] > W) continue;
            pool.push_back(pi);
            if((int)pool.size() >= K) break;
        }
        
        int bestWaste = INT_MAX, bestC = INT_MIN, bestTop = INT_MAX, bestX0 = INT_MAX;
        int bestPi=-1, bestOri=-1, bestY0=-1;
        
        const int ddx[4] = {1,-1,0,0};
        const int ddy[4] = {0,0,1,-1};
        
        for(int pi : pool){
            for(int oi=0; oi<8; oi++){
                Orient& o = orients[pi][oi];
                if(o.w > w_gap) continue;
                if(min_h + o.h > W) continue;
                
                int y0 = min_h;
                
                // pmask
                // small; build on the fly
                static vector<vector<char>> pmask;
                pmask.assign(o.w, vector<char>(o.h, 0));
                for(auto& c : o.cells) pmask[c.first][c.second] = 1;
                
                for(int x0 = lo; x0 <= hi - o.w; x0++){
                    int waste = (x0 - lo) + (hi - x0 - o.w);
                    int top = y0 + o.h;
                    if(waste > bestWaste) continue;
                    
                    int C = 0;
                    for(auto& c : o.cells){
                        int cx = c.first, cy = c.second;
                        for(int k=0;k<4;k++){
                            int nx = cx + ddx[k], ny = cy + ddy[k];
                            if(nx>=0 && nx<o.w && ny>=0 && ny<o.h && pmask[nx][ny]) continue;
                            int gx = x0+cx+ddx[k], gy = y0+cy+ddy[k];
                            if(gx<0 || gx>=W || gy<0 || gy>=W) C++;
                            else {
                                if((col_bits[gx][gy>>6] >> (gy&63)) & 1ULL) C++;
                            }
                        }
                    }
                    
                    bool better = false;
                    if(waste < bestWaste) better = true;
                    else if(waste == bestWaste){
                        if(C > bestC) better = true;
                        else if(C == bestC){
                            if(top < bestTop) better = true;
                            else if(top == bestTop && x0 < bestX0) better = true;
                        }
                    }
                    if(better){
                        bestWaste=waste; bestC=C; bestTop=top; bestX0=x0;
                        bestPi=pi; bestOri=oi; bestY0=y0;
                    }
                }
            }
        }
        
        if(bestPi >= 0){
            Orient& o = orients[bestPi][bestOri];
            for(auto& c : o.cells){
                int gx = bestX0+c.first, gy = bestY0+c.second;
                col_bits[gx][gy>>6] |= (1ULL << (gy&63));
            }
            placed[bestPi] = 1;
            remaining--;
            for(int gx = bestX0; gx < bestX0 + o.w; gx++){
                int dx = gx - bestX0;
                if(o.max_dy[dx] >= 0){
                    int v = bestY0 + o.max_dy[dx] + 1;
                    if(v > skyline[gx]) skyline[gx] = v;
                }
            }
            int f = bestOri/4, r = bestOri%4;
            tF[bestPi] = f; tR[bestPi] = r;
            tX[bestPi] = bestX0 - o.min_tx;
            tY[bestPi] = bestY0 - o.min_ty;
        } else {
            int new_h = min(h_left, h_right);
            if(new_h == INT_MAX || new_h >= W) return false;
            for(int c=lo;c<hi;c++) skyline[c] = new_h;
        }
    }
    return true;
}

int main(){
    if(!(cin >> n)) return 0;
    vector<vector<pair<int,int>>> pieces(n);
    ks.assign(n, 0);
    int T = 0;
    for(int i=0;i<n;i++){
        cin >> ks[i];
        pieces[i].resize(ks[i]);
        for(int j=0;j<ks[i];j++){
            cin >> pieces[i][j].first >> pieces[i][j].second;
        }
        T += ks[i];
    }
    
    orients.assign(n, vector<Orient>(8));
    vector<int> maxArea(n, 0);
    min_w_orient.assign(n, INT_MAX);
    min_h_orient.assign(n, INT_MAX);
    for(int i=0;i<n;i++){
        for(int f=0;f<2;f++){
            for(int r=0;r<4;r++){
                int idx = f*4 + r;
                Orient& o = orients[i][idx];
                vector<pair<int,int>> tc(ks[i]);
                for(int j=0;j<ks[i];j++){
                    int x = pieces[i][j].first, y = pieces[i][j].second;
                    if(f==1) x = -x;
                    for(int rr=0;rr<r;rr++){
                        int nx = y, ny = -x;
                        x = nx; y = ny;
                    }
                    tc[j] = {x,y};
                }
                int mnx=INT_MAX, mny=INT_MAX, mxx=INT_MIN, mxy=INT_MIN;
                for(auto& p: tc){
                    mnx=min(mnx,p.first); mny=min(mny,p.second);
                    mxx=max(mxx,p.first); mxy=max(mxy,p.second);
                }
                o.min_tx = mnx; o.min_ty = mny;
                o.w = mxx-mnx+1; o.h = mxy-mny+1;
                o.cells.resize(ks[i]);
                for(int j=0;j<ks[i];j++){
                    o.cells[j] = {tc[j].first-mnx, tc[j].second-mny};
                }
                o.min_dy.assign(o.w, INT_MAX);
                o.max_dy.assign(o.w, -1);
                o.pattern_dx.assign(o.w, {});
                for(auto& c: o.cells){
                    o.min_dy[c.first] = min(o.min_dy[c.first], c.second);
                    o.max_dy[c.first] = max(o.max_dy[c.first], c.second);
                    o.pattern_dx[c.first].push_back(c.second);
                }
                for(int c=0;c<o.w;c++) if(o.max_dy[c]>=0) o.cols.push_back(c);
                maxArea[i] = max(maxArea[i], o.w*o.h);
                min_w_orient[i] = min(min_w_orient[i], o.w);
                min_h_orient[i] = min(min_h_orient[i], o.h);
            }
        }
    }
    
    priorityOrder.resize(n);
    iota(priorityOrder.begin(), priorityOrder.end(), 0);
    sort(priorityOrder.begin(), priorityOrder.end(), [&](int a, int b){
        if(maxArea[a] != maxArea[b]) return maxArea[a] > maxArea[b];
        return ks[a] > ks[b];
    });
    
    W = (int)ceil(sqrt((double)T));
    if(W<1) W=1;
    
    vector<int> outX(n), outY(n), outR(n), outF(n);
    
    while(true){
        NW = (W + 63) / 64;
        vector<vector<uint64_t>> col_bits(W, vector<uint64_t>(NW, 0));
        vector<int> tX(n), tY(n), tR(n), tF(n);
        
        if(tryPack(col_bits, tX, tY, tR, tF)){
            outX=tX; outY=tY; outR=tR; outF=tF;
            break;
        }
        W++;
    }
    
    cout << W << " " << W << "\n";
    for(int i=0;i<n;i++){
        cout << outX[i] << " " << outY[i] << " " << outR[i] << " " << outF[i] << "\n";
    }
    return 0;
}