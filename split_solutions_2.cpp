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

static vector<int> setMinus(const vector<int>& A, const vector<int>& B){
    set<int> Bs(B.begin(), B.end());
    vector<int> res;
    for(int x : A) if(!Bs.count(x)) res.push_back(x);
    return res;
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
    
    auto singleBinarySearch = [&](int v, vector<int> C) -> int {
        while(C.size() > 1){
            int sz = C.size();
            int half = (sz + 1) / 2;
            vector<int> L(C.begin(), C.begin() + half);
            
            vector<int> q(n+1, v_anchor);
            for(int j : L) q[j] = v;
            
            int r = doQuery(q);
            int indicator = r - 1;
            
            if(indicator == 1){
                C = L;
            } else {
                C = setMinus(C, L);
            }
        }
        return C[0];
    };
    
    size_t idx = 0;
    
    while(idx + 2 < V.size()){
        int a = V[idx], b = V[idx+1], c = V[idx+2];
        idx += 3;
        
        vector<int> Ca = G;
        vector<int> Cb = G;
        vector<int> Cc = G;
        
        // Phase A: single query
        int sz = Ca.size();
        if(sz < 3){
            // fallback: three independent binary searches
            int pa = singleBinarySearch(a, Ca);
            hidden[pa] = a;
            Cb = setMinus(Cb, {pa});
            int pb = singleBinarySearch(b, Cb);
            hidden[pb] = b;
            Cc = setMinus(Cc, {pa, pb});
            int pc = singleBinarySearch(c, Cc);
            hidden[pc] = c;
            G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x == pa || x == pb || x == pc; }), G.end());
            continue;
        }
        
        int t1 = sz / 3, t2 = 2 * sz / 3;
        vector<int> Sa(Ca.begin(), Ca.begin() + t1);
        vector<int> Sb(Ca.begin() + t1, Ca.begin() + t2);
        vector<int> Sc(Ca.begin() + t2, Ca.end());
        
        vector<int> q(n+1, v_anchor);
        for(int j : Sa) q[j] = a;
        for(int j : Sb) q[j] = b;
        for(int j : Sc) q[j] = c;
        
        int r = doQuery(q);
        int sum = r - 1;
        
        if(sum == 3){
            Ca = Sa;
            Cb = Sb;
            Cc = Sc;
        } else if(sum == 0){
            Ca = Sb; Ca.insert(Ca.end(), Sc.begin(), Sc.end());
            Cb = Sa; Cb.insert(Cb.end(), Sc.begin(), Sc.end());
            Cc = Sa; Cc.insert(Cc.end(), Sb.begin(), Sb.end());
        } else {
            // sum 1 or 2: disambiguate
            vector<int> qA(n+1, v_anchor);
            for(int j : Sa) qA[j] = a;
            int rA = doQuery(qA);
            int ind_a = rA - 1;
            
            vector<int> qB(n+1, v_anchor);
            for(int j : Sb) qB[j] = b;
            int rB = doQuery(qB);
            int ind_b = rB - 1;
            
            int ind_c = sum - ind_a - ind_b;
            
            if(ind_a == 1) Ca = Sa;
            else { Ca = Sb; Ca.insert(Ca.end(), Sc.begin(), Sc.end()); }
            
            if(ind_b == 1) Cb = Sb;
            else { Cb = Sa; Cb.insert(Cb.end(), Sc.begin(), Sc.end()); }
            
            if(ind_c == 1) Cc = Sc;
            else { Cc = Sa; Cc.insert(Cc.end(), Sb.begin(), Sb.end()); }
        }
        
        // Phase B: three independent binary searches
        int pa = singleBinarySearch(a, Ca);
        int pb = singleBinarySearch(b, Cb);
        int pc = singleBinarySearch(c, Cc);
        
        hidden[pa] = a;
        hidden[pb] = b;
        hidden[pc] = c;
        
        G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x == pa || x == pb || x == pc; }), G.end());
    }
    
    // Trailing remainder of 2: reuse pair joint code
    while(idx + 1 < V.size()){
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
        int pos = singleBinarySearch(v, G);
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