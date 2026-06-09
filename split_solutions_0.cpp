#include <bits/stdc++.h>
using namespace std;

struct Orient {
    int w, h;
    vector<pair<int,int>> cells;
    vector<int> cols;
    vector<int> min_dy;
    vector<int> max_dy;
    int min_tx, min_ty;
};

int main(){
    int n;
    if(!(cin >> n)) return 0;
    vector<vector<pair<int,int>>> pieces(n);
    vector<int> ks(n);
    int T = 0;
    for(int i=0;i<n;i++){
        cin >> ks[i];
        pieces[i].resize(ks[i]);
        for(int j=0;j<ks[i];j++){
            cin >> pieces[i][j].first >> pieces[i][j].second;
        }
        T += ks[i];
    }
    
    vector<vector<Orient>> orients(n, vector<Orient>(8));
    vector<int> maxArea(n, 0);
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
                for(auto& c: o.cells){
                    o.min_dy[c.first] = min(o.min_dy[c.first], c.second);
                    o.max_dy[c.first] = max(o.max_dy[c.first], c.second);
                }
                for(int c=0;c<o.w;c++) if(o.max_dy[c]>=0) o.cols.push_back(c);
                maxArea[i] = max(maxArea[i], o.w*o.h);
            }
        }
    }
    
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b){
        if(maxArea[a] != maxArea[b]) return maxArea[a] > maxArea[b];
        return ks[a] > ks[b];
    });
    
    int W = (int)ceil(sqrt((double)T));
    if(W<1) W=1;
    
    vector<int> outX(n), outY(n), outR(n), outF(n);
    
    while(true){
        vector<int> skyline(W, 0);
        vector<vector<int>> occ(W, vector<int>(W, 0));
        bool ok = true;
        vector<int> tX(n), tY(n), tR(n), tF(n);
        
        for(int pi : order){
            int bestScore1 = INT_MAX, bestScore2 = INT_MAX, bestScore3 = INT_MAX;
            int bestOri=-1, bestX0=-1, bestY0=-1;
            for(int oi=0; oi<8; oi++){
                Orient& o = orients[pi][oi];
                if(o.w > W) continue;
                if(o.h > W) continue;
                for(int x0=0; x0<=W-o.w; x0++){
                    int y0 = 0;
                    for(int dx : o.cols){
                        int cand = skyline[x0+dx] - o.min_dy[dx];
                        if(cand > y0) y0 = cand;
                    }
                    if(y0 + o.h > W) continue;
                    bool good = true;
                    for(auto& c : o.cells){
                        if(occ[x0+c.first][y0+c.second]) { good=false; break; }
                    }
                    if(!good) continue;
                    int s1 = y0+o.h, s2=y0, s3=x0;
                    bool better = false;
                    if(s1 < bestScore1) better = true;
                    else if(s1 == bestScore1){
                        if(s2 < bestScore2) better = true;
                        else if(s2 == bestScore2 && s3 < bestScore3) better = true;
                    }
                    if(better){
                        bestScore1=s1; bestScore2=s2; bestScore3=s3;
                        bestOri=oi; bestX0=x0; bestY0=y0;
                    }
                }
            }
            if(bestOri<0){ ok=false; break; }
            Orient& o = orients[pi][bestOri];
            for(auto& c : o.cells){
                occ[bestX0+c.first][bestY0+c.second] = 1;
            }
            for(int dx : o.cols){
                int ns = bestY0 + o.max_dy[dx] + 1;
                if(ns > skyline[bestX0+dx]) skyline[bestX0+dx] = ns;
            }
            int f = bestOri/4, r = bestOri%4;
            tF[pi] = f; tR[pi] = r;
            tX[pi] = bestX0 - o.min_tx;
            tY[pi] = bestY0 - o.min_ty;
        }
        
        if(ok){
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