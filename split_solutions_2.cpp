#include <bits/stdc++.h>
using namespace std;

static int n;
static int v_anchor;

int doQuery(const vector<int>& q){
    cout << 0;
    for(int j = 1; j <= n; j++) cout << ' ' << q[j];
    cout << '\n';
    cout.flush();
    int r;
    cin >> r;
    return r;
}

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    cin >> n;
    
    if(n == 1){
        cout << "1 1" << '\n';
        cout.flush();
        return 0;
    }
    
    vector<int> hidden(n+1, 0);
    int p_anchor = -1;
    v_anchor = -1;
    
    auto fallbackScan = [&](){
        for(int k = 1; k <= n; k++){
            int i;
            if(k % 2 == 1) i = (k + 1) / 2;
            else i = n + 1 - (k / 2);
            
            vector<int> q(n+1, 1);
            q[i] = 2;
            int a = doQuery(q);
            if(a == 0){
                p_anchor = i;
                v_anchor = 1;
                return;
            } else if(a == 2){
                p_anchor = i;
                v_anchor = 2;
                return;
            }
        }
    };
    
    if(n < 30){
        fallbackScan();
    } else {
        int k = max(2, (int)floor(sqrt((double)n)));
        vector<int> found_chunk;
        bool found = false;
        for(int start = 1; start <= n; start += k){
            int end = min(n, start + k - 1);
            vector<int> C;
            for(int j = start; j <= end; j++) C.push_back(j);
            
            vector<int> q(n+1, 1);
            for(int j : C) q[j] = 2;
            int a = doQuery(q);
            if(a == 0){
                found_chunk = C;
                v_anchor = 1;
                found = true;
                break;
            } else if(a == 2){
                found_chunk = C;
                v_anchor = 2;
                found = true;
                break;
            }
        }
        
        if(!found){
            fallbackScan();
        } else {
            vector<int> C = found_chunk;
            if(v_anchor == 1){
                while(C.size() > 1){
                    int sz = C.size();
                    int half = (sz + 1) / 2;
                    vector<int> L(C.begin(), C.begin() + half);
                    vector<int> R(C.begin() + half, C.end());
                    
                    vector<int> q(n+1, 1);
                    for(int j : R) q[j] = 2;
                    int a = doQuery(q);
                    if(a == 1) C = L;
                    else C = R;
                }
            } else {
                while(C.size() > 1){
                    int sz = C.size();
                    int half = (sz + 1) / 2;
                    vector<int> L(C.begin(), C.begin() + half);
                    vector<int> R(C.begin() + half, C.end());
                    
                    vector<int> q(n+1, 1);
                    for(int j : L) q[j] = 2;
                    int a = doQuery(q);
                    if(a == 2) C = L;
                    else C = R;
                }
            }
            p_anchor = C[0];
        }
    }
    
    hidden[p_anchor] = v_anchor;
    
    vector<int> G;
    for(int i = 1; i <= n; i++){
        if(i != p_anchor) G.push_back(i);
    }
    
    vector<int> V;
    for(int v = 1; v <= n-1; v++){
        if(v != v_anchor) V.push_back(v);
    }
    
    size_t idx = 0;
    while(idx + 1 < V.size()){
        // Triple-end direct enumeration branch
        if(n >= 30 && idx + 2 == V.size() && G.size() == 3){
            int a = V[idx], b = V[idx+1];
            int g1 = G[0], g2 = G[1], g3 = G[2];
            
            vector<int> q1(n+1, v_anchor);
            q1[g1] = a;
            q1[g2] = b;
            // q1[g3] stays v_anchor
            int r1 = doQuery(q1);
            int s1 = r1 - 1;
            
            if(s1 == 2){
                hidden[g1] = a;
                hidden[g2] = b;
                hidden[g3] = n;
                G.clear();
                idx = V.size();
                break;
            } else if(s1 == 1){
                vector<int> q2(n+1, v_anchor);
                q2[g1] = a;
                q2[g2] = a;
                // q2[g3] stays v_anchor
                int r2 = doQuery(q2);
                int s2 = r2 - 1;
                if(s2 == 1){
                    // (a, n, b)
                    hidden[g1] = a;
                    hidden[g2] = n;
                    hidden[g3] = b;
                } else {
                    // (n, b, a)
                    hidden[g1] = n;
                    hidden[g2] = b;
                    hidden[g3] = a;
                }
                G.clear();
                idx = V.size();
                break;
            } else {
                // s1 == 0
                vector<int> q2(n+1, v_anchor);
                q2[g1] = b;
                q2[g2] = a;
                int r2 = doQuery(q2);
                int s2 = r2 - 1;
                if(s2 == 2){
                    // (b, a, n)
                    hidden[g1] = b;
                    hidden[g2] = a;
                    hidden[g3] = n;
                    G.clear();
                    idx = V.size();
                    break;
                } else {
                    // s2 == 1
                    vector<int> q3(n+1, v_anchor);
                    q3[g1] = b;
                    int r3 = doQuery(q3);
                    int s3 = r3 - 1;
                    if(s3 == 1){
                        // (b, n, a)
                        hidden[g1] = b;
                        hidden[g2] = n;
                        hidden[g3] = a;
                    } else {
                        // (n, a, b)
                        hidden[g1] = n;
                        hidden[g2] = a;
                        hidden[g3] = b;
                    }
                    G.clear();
                    idx = V.size();
                    break;
                }
            }
        }
        
        int a = V[idx], b = V[idx+1];
        idx += 2;
        
        vector<int> Ca = G;
        vector<int> Cb = G;
        
        while(Ca == Cb && Ca.size() > 1){
            int sz = Ca.size();
            int m = (sz + 1) / 2;
            vector<int> Sa(Ca.begin(), Ca.begin() + m);
            vector<int> Sb(Ca.begin() + m, Ca.end());
            
            vector<int> q(n+1, v_anchor);
            for(int j : Sa) q[j] = a;
            for(int j : Sb) q[j] = b;
            
            int r = doQuery(q);
            int sum = r - 1;
            
            if(sum == 0){
                Ca = Sb;
                Cb = Sa;
            } else if(sum == 2){
                Ca = Sa;
                Cb = Sb;
            } else {
                vector<int> q2(n+1, v_anchor);
                for(int j : Sa) q2[j] = a;
                int r2 = doQuery(q2);
                int ba = r2 - 1;
                if(ba == 1){
                    Ca = Sa;
                    Cb = Sa;
                } else {
                    Ca = Sb;
                    Cb = Sb;
                }
            }
        }
        
        while(Ca.size() > 1 || Cb.size() > 1){
            vector<int> Sa, Sb;
            if(Ca.size() > 1){
                int half = (Ca.size() + 1) / 2;
                Sa.assign(Ca.begin(), Ca.begin() + half);
            }
            if(Cb.size() > 1){
                int half = (Cb.size() + 1) / 2;
                Sb.assign(Cb.begin(), Cb.begin() + half);
            }
            
            vector<int> q(n+1, v_anchor);
            for(int j : Sa) q[j] = a;
            for(int j : Sb) q[j] = b;
            
            int r = doQuery(q);
            int sum = r - 1;
            
            auto setMinus = [](const vector<int>& A, const vector<int>& B){
                set<int> Bs(B.begin(), B.end());
                vector<int> res;
                for(int x : A) if(!Bs.count(x)) res.push_back(x);
                return res;
            };
            
            if(Sa.empty()){
                if(sum == 1) Cb = Sb;
                else Cb = setMinus(Cb, Sb);
            } else if(Sb.empty()){
                if(sum == 1) Ca = Sa;
                else Ca = setMinus(Ca, Sa);
            } else {
                if(sum == 0){
                    Ca = setMinus(Ca, Sa);
                    Cb = setMinus(Cb, Sb);
                } else if(sum == 2){
                    Ca = Sa;
                    Cb = Sb;
                } else {
                    vector<int> q2(n+1, v_anchor);
                    for(int j : Sa) q2[j] = a;
                    int r2 = doQuery(q2);
                    int ba = r2 - 1;
                    int bb = 1 - ba;
                    if(ba == 1) Ca = Sa;
                    else Ca = setMinus(Ca, Sa);
                    if(bb == 1) Cb = Sb;
                    else Cb = setMinus(Cb, Sb);
                }
            }
        }
        
        hidden[Ca[0]] = a;
        hidden[Cb[0]] = b;
        int pa = Ca[0], pb = Cb[0];
        G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x == pa || x == pb; }), G.end());
    }
    
    if(idx < V.size()){
        int v = V[idx];
        vector<int> C = G;
        while(C.size() > 1){
            int sz = C.size();
            int half = (sz + 1) / 2;
            vector<int> L(C.begin(), C.begin() + half);
            set<int> Lset(L.begin(), L.end());
            
            vector<int> q(n+1, v_anchor);
            for(int j : L) q[j] = v;
            
            int a = doQuery(q);
            int indicator = a - 1;
            
            if(indicator == 1){
                C = L;
            } else {
                vector<int> newC;
                for(int x : C) if(!Lset.count(x)) newC.push_back(x);
                C = newC;
            }
        }
        int pos = C[0];
        hidden[pos] = v;
        G.erase(remove(G.begin(), G.end(), pos), G.end());
    }
    
    for(int i = 1; i <= n; i++){
        if(hidden[i] == 0){
            hidden[i] = n;
            break;
        }
    }
    
    cout << 1;
    for(int i = 1; i <= n; i++) cout << ' ' << hidden[i];
    cout << '\n';
    cout.flush();
    
    return 0;
}