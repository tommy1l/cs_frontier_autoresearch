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

bool tryPack(const vector<int>& order, vector<vector<uint64_t>>& col_bits,
             vector<int>& tX, vector<int>& tY, vector<int>& tR, vector<int>& tF){
    for(auto& cb : col_bits) fill(cb.begin(), cb.end(), 0ULL);
    
    for(int pi : order){
        int bestS1 = INT_MAX, bestC = INT_MIN, bestS2 = INT_MAX, bestS3 = INT_MAX;
        int bestOri=-1, bestX0=-1, bestY0=-1;
        for(int oi=0; oi<8; oi++){
            Orient& o = orients[pi][oi];
            if(o.w > W) continue;
            if(o.h > W) continue;
            vector<vector<char>> pmask(o.w, vector<char>(o.h, 0));
            for(auto& c : o.cells) pmask[c.first][c.second] = 1;
            
            vector<uint64_t> total_blocked(NW, 0);
            
            for(int x0=0; x0<=W-o.w; x0++){
                fill(total_blocked.begin(), total_blocked.end(), 0ULL);
                
                for(int dx : o.cols){
                    const auto& colb = col_bits[x0+dx];
                    for(int dy : o.pattern_dx[dx]){
                        int wshift = dy / 64;
                        int bshift = dy % 64;
                        if(bshift == 0){
                            for(int w=0; w+wshift<NW; w++){
                                total_blocked[w] |= colb[w+wshift];
                            }
                        } else {
                            for(int w=0; w+wshift<NW; w++){
                                uint64_t lo = colb[w+wshift] >> bshift;
                                uint64_t hi = 0;
                                if(w+wshift+1 < NW) hi = colb[w+wshift+1] << (64-bshift);
                                total_blocked[w] |= (lo | hi);
                            }
                        }
                    }
                }
                
                int maxY0 = W - o.h;
                int firstBlock = maxY0 + 1;
                if(firstBlock < 0) continue;
                for(int w=0; w<NW; w++){
                    int base = w*64;
                    if(base >= firstBlock){
                        total_blocked[w] = ~0ULL;
                    } else if(base + 64 > firstBlock){
                        int bits = firstBlock - base;
                        uint64_t mask = (bits>=64)?0ULL:(~((1ULL<<bits)-1));
                        total_blocked[w] |= mask;
                    }
                }
                
                int y0 = -1;
                for(int w=0; w<NW; w++){
                    uint64_t v = ~total_blocked[w];
                    if(v != 0){
                        int b = __builtin_ctzll(v);
                        y0 = w*64 + b;
                        break;
                    }
                }
                if(y0 < 0 || y0 > maxY0) continue;
                
                int C = 0;
                const int ddx[4] = {1,-1,0,0};
                const int ddy[4] = {0,0,1,-1};
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
                int s1 = y0+o.h, s2=y0, s3=x0;
                bool better = false;
                if(s1 < bestS1) better = true;
                else if(s1 == bestS1){
                    if(C > bestC) better = true;
                    else if(C == bestC){
                        if(s2 < bestS2) better = true;
                        else if(s2 == bestS2 && s3 < bestS3) better = true;
                    }
                }
                if(better){
                    bestS1=s1; bestC=C; bestS2=s2; bestS3=s3;
                    bestOri=oi; bestX0=x0; bestY0=y0;
                }
            }
        }
        if(bestOri<0){ return false; }
        Orient& o = orients[pi][bestOri];
        for(auto& c : o.cells){
            int gx = bestX0+c.first, gy = bestY0+c.second;
            col_bits[gx][gy>>6] |= (1ULL << (gy&63));
        }
        int f = bestOri/4, r = bestOri%4;
        tF[pi] = f; tR[pi] = r;
        tX[pi] = bestX0 - o.min_tx;
        tY[pi] = bestY0 - o.min_ty;
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
    vector<int> maxDim(n, 0);
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
                maxDim[i] = max(maxDim[i], max(o.w, o.h));
            }
        }
    }
    
    vector<int> order1(n), order2(n);
    iota(order1.begin(), order1.end(), 0);
    sort(order1.begin(), order1.end(), [&](int a, int b){
        if(maxArea[a] != maxArea[b]) return maxArea[a] > maxArea[b];
        return ks[a] > ks[b];
    });
    iota(order2.begin(), order2.end(), 0);
    sort(order2.begin(), order2.end(), [&](int a, int b){
        if(maxDim[a] != maxDim[b]) return maxDim[a] > maxDim[b];
        if(ks[a] != ks[b]) return ks[a] > ks[b];
        return maxArea[a] > maxArea[b];
    });
    
    W = (int)ceil(sqrt((double)T));
    if(W<1) W=1;
    
    vector<int> outX(n), outY(n), outR(n), outF(n);
    
    while(true){
        NW = (W + 63) / 64;
        vector<vector<uint64_t>> col_bits(W, vector<uint64_t>(NW, 0));
        vector<int> tX(n), tY(n), tR(n), tF(n);
        
        if(tryPack(order1, col_bits, tX, tY, tR, tF)){
            outX=tX; outY=tY; outR=tR; outF=tF;
            break;
        }
        if(tryPack(order2, col_bits, tX, tY, tR, tF)){
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