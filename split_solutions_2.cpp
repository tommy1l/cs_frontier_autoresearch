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
            int h = k / 2;
            if(h >= 1){
                vector<vector<int>> shifted_chunks;
                {
                    vector<int> first;
                    for(int j = 1; j <= h && j <= n; j++) first.push_back(j);
                    shifted_chunks.push_back(first);
                    int j = 0;
                    while(true){
                        int s = h + j * k + 1;
                        int e = min(n, h + (j + 1) * k);
                        if(s > n) break;
                        vector<int> C;
                        for(int p = s; p <= e; p++) C.push_back(p);
                        shifted_chunks.push_back(C);
                        if(e >= n) break;
                        j++;
                    }
                }
                
                for(auto& Cprime : shifted_chunks){
                    if(Cprime.empty()) continue;
                    vector<int> q(n+1, 1);
                    for(int j : Cprime) q[j] = 2;
                    int a = doQuery(q);
                    if(a == 0){
                        found_chunk = Cprime;
                        v_anchor = 1;
                        found = true;
                        break;
                    } else if(a == 2){
                        found_chunk = Cprime;
                        v_anchor = 2;
                        found = true;
                        break;
                    }
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
    
    // Process groups of 4
    while(idx + 3 < V.size()){
        int a = V[idx], b = V[idx+1], c = V[idx+2], d = V[idx+3];
        idx += 4;
        
        int gsz = G.size();
        int base = gsz / 4;
        int rem = gsz % 4;
        int s1 = base + (rem > 0 ? 1 : 0);
        int s2 = base + (rem > 1 ? 1 : 0);
        int s3 = base + (rem > 2 ? 1 : 0);
        int s4 = base;
        
        vector<int> Q1(G.begin(), G.begin() + s1);
        vector<int> Q2(G.begin() + s1, G.begin() + s1 + s2);
        vector<int> Q3(G.begin() + s1 + s2, G.begin() + s1 + s2 + s3);
        vector<int> Q4(G.begin() + s1 + s2 + s3, G.end());
        
        // Initial query
        vector<int> q(n+1, v_anchor);
        for(int j : Q1) q[j] = a;
        for(int j : Q2) q[j] = b;
        for(int j : Q3) q[j] = c;
        for(int j : Q4) q[j] = d;
        int r = doQuery(q);
        int sum = r - 1;
        
        int Ia, Ib, Ic, Id;
        if(sum == 4){ Ia=Ib=Ic=Id=1; }
        else if(sum == 0){ Ia=Ib=Ic=Id=0; }
        else {
            // query a only
            vector<int> q2(n+1, v_anchor);
            for(int j : Q1) q2[j] = a;
            int r2 = doQuery(q2);
            Ia = r2 - 1;
            int remaining = sum - Ia;
            if(remaining == 0){ Ib=Ic=Id=0; }
            else if(remaining == 3){ Ib=Ic=Id=1; }
            else {
                vector<int> q3(n+1, v_anchor);
                for(int j : Q2) q3[j] = b;
                int r3 = doQuery(q3);
                Ib = r3 - 1;
                int remaining2 = remaining - Ib;
                if(remaining2 == 0){ Ic=Id=0; }
                else if(remaining2 == 2){ Ic=Id=1; }
                else {
                    vector<int> q4(n+1, v_anchor);
                    for(int j : Q3) q4[j] = c;
                    int r4 = doQuery(q4);
                    Ic = r4 - 1;
                    Id = 1 - Ic;
                }
            }
        }
        
        vector<int> Ca = Ia ? Q1 : setMinus(G, Q1);
        vector<int> Cb = Ib ? Q2 : setMinus(G, Q2);
        vector<int> Cc = Ic ? Q3 : setMinus(G, Q3);
        vector<int> Cd = Id ? Q4 : setMinus(G, Q4);
        
        // Halving loop
        while(Ca.size() > 1 || Cb.size() > 1 || Cc.size() > 1 || Cd.size() > 1){
            vector<int> Sa, Sb, Sc, Sd;
            vector<int> A; // values participating
            if(Ca.size() > 1){
                int h = (Ca.size() + 1) / 2;
                Sa.assign(Ca.begin(), Ca.begin() + h);
                A.push_back(0);
            }
            if(Cb.size() > 1){
                int h = (Cb.size() + 1) / 2;
                Sb.assign(Cb.begin(), Cb.begin() + h);
                A.push_back(1);
            }
            if(Cc.size() > 1){
                int h = (Cc.size() + 1) / 2;
                Sc.assign(Cc.begin(), Cc.begin() + h);
                A.push_back(2);
            }
            if(Cd.size() > 1){
                int h = (Cd.size() + 1) / 2;
                Sd.assign(Cd.begin(), Cd.begin() + h);
                A.push_back(3);
            }
            
            vector<int> q(n+1, v_anchor);
            for(int j : Sa) q[j] = a;
            for(int j : Sb) q[j] = b;
            for(int j : Sc) q[j] = c;
            for(int j : Sd) q[j] = d;
            int r = doQuery(q);
            int sum = r - 1;
            int Asz = A.size();
            
            vector<int> Ind(4, 0); // for indices 0..3
            
            if(sum == 0){
                // all 0
            } else if(sum == Asz){
                for(int idv : A) Ind[idv] = 1;
            } else {
                int remaining = sum;
                int processed = 0;
                for(int ai = 0; ai < Asz; ai++){
                    int idv = A[ai];
                    int rest = Asz - ai - 1; // remaining after this one
                    if(remaining == 0){
                        // all rest 0
                        break;
                    }
                    if(remaining == Asz - ai){
                        // all remaining (including current) are 1
                        for(int aj = ai; aj < Asz; aj++) Ind[A[aj]] = 1;
                        break;
                    }
                    if(ai == Asz - 1){
                        // last one
                        Ind[idv] = remaining;
                        break;
                    }
                    // query just this one
                    vector<int> qq(n+1, v_anchor);
                    if(idv == 0) for(int j : Sa) qq[j] = a;
                    else if(idv == 1) for(int j : Sb) qq[j] = b;
                    else if(idv == 2) for(int j : Sc) qq[j] = c;
                    else for(int j : Sd) qq[j] = d;
                    int rr = doQuery(qq);
                    int iv = rr - 1;
                    Ind[idv] = iv;
                    remaining -= iv;
                }
            }
            
            if(!Sa.empty()){
                if(Ind[0]) Ca = Sa; else Ca = setMinus(Ca, Sa);
            }
            if(!Sb.empty()){
                if(Ind[1]) Cb = Sb; else Cb = setMinus(Cb, Sb);
            }
            if(!Sc.empty()){
                if(Ind[2]) Cc = Sc; else Cc = setMinus(Cc, Sc);
            }
            if(!Sd.empty()){
                if(Ind[3]) Cd = Sd; else Cd = setMinus(Cd, Sd);
            }
        }
        
        hidden[Ca[0]] = a;
        hidden[Cb[0]] = b;
        hidden[Cc[0]] = c;
        hidden[Cd[0]] = d;
        int pa = Ca[0], pb = Cb[0], pc = Cc[0], pd = Cd[0];
        G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x==pa||x==pb||x==pc||x==pd; }), G.end());
    }
    
    // Tail: 3 remaining
    if(idx + 2 < V.size()){
        int a = V[idx], b = V[idx+1], c = V[idx+2];
        idx += 3;
        
        int gsz = G.size();
        int base = gsz / 3;
        int rem = gsz % 3;
        int s1 = base + (rem > 0 ? 1 : 0);
        int s2 = base + (rem > 1 ? 1 : 0);
        int s3 = base;
        
        vector<int> Q1(G.begin(), G.begin() + s1);
        vector<int> Q2(G.begin() + s1, G.begin() + s1 + s2);
        vector<int> Q3(G.begin() + s1 + s2, G.end());
        
        vector<int> q(n+1, v_anchor);
        for(int j : Q1) q[j] = a;
        for(int j : Q2) q[j] = b;
        for(int j : Q3) q[j] = c;
        int r = doQuery(q);
        int sum = r - 1;
        
        int Ia, Ib, Ic;
        if(sum == 3){ Ia=Ib=Ic=1; }
        else if(sum == 0){ Ia=Ib=Ic=0; }
        else {
            vector<int> q2(n+1, v_anchor);
            for(int j : Q1) q2[j] = a;
            int r2 = doQuery(q2);
            Ia = r2 - 1;
            int remaining = sum - Ia;
            if(remaining == 0){ Ib=Ic=0; }
            else if(remaining == 2){ Ib=Ic=1; }
            else {
                vector<int> q3(n+1, v_anchor);
                for(int j : Q2) q3[j] = b;
                int r3 = doQuery(q3);
                Ib = r3 - 1;
                Ic = remaining - Ib;
            }
        }
        
        vector<int> Ca = Ia ? Q1 : setMinus(G, Q1);
        vector<int> Cb = Ib ? Q2 : setMinus(G, Q2);
        vector<int> Cc = Ic ? Q3 : setMinus(G, Q3);
        
        while(Ca.size() > 1 || Cb.size() > 1 || Cc.size() > 1){
            vector<int> Sa, Sb, Sc;
            vector<int> A;
            if(Ca.size() > 1){
                int h = (Ca.size() + 1) / 2;
                Sa.assign(Ca.begin(), Ca.begin() + h);
                A.push_back(0);
            }
            if(Cb.size() > 1){
                int h = (Cb.size() + 1) / 2;
                Sb.assign(Cb.begin(), Cb.begin() + h);
                A.push_back(1);
            }
            if(Cc.size() > 1){
                int h = (Cc.size() + 1) / 2;
                Sc.assign(Cc.begin(), Cc.begin() + h);
                A.push_back(2);
            }
            
            vector<int> q(n+1, v_anchor);
            for(int j : Sa) q[j] = a;
            for(int j : Sb) q[j] = b;
            for(int j : Sc) q[j] = c;
            int r = doQuery(q);
            int sum = r - 1;
            int Asz = A.size();
            
            vector<int> Ind(3, 0);
            if(sum == 0){}
            else if(sum == Asz){
                for(int idv : A) Ind[idv] = 1;
            } else {
                int remaining = sum;
                for(int ai = 0; ai < Asz; ai++){
                    int idv = A[ai];
                    if(remaining == 0) break;
                    if(remaining == Asz - ai){
                        for(int aj = ai; aj < Asz; aj++) Ind[A[aj]] = 1;
                        break;
                    }
                    if(ai == Asz - 1){
                        Ind[idv] = remaining;
                        break;
                    }
                    vector<int> qq(n+1, v_anchor);
                    if(idv == 0) for(int j : Sa) qq[j] = a;
                    else if(idv == 1) for(int j : Sb) qq[j] = b;
                    else for(int j : Sc) qq[j] = c;
                    int rr = doQuery(qq);
                    int iv = rr - 1;
                    Ind[idv] = iv;
                    remaining -= iv;
                }
            }
            
            if(!Sa.empty()){ if(Ind[0]) Ca = Sa; else Ca = setMinus(Ca, Sa); }
            if(!Sb.empty()){ if(Ind[1]) Cb = Sb; else Cb = setMinus(Cb, Sb); }
            if(!Sc.empty()){ if(Ind[2]) Cc = Sc; else Cc = setMinus(Cc, Sc); }
        }
        
        hidden[Ca[0]] = a;
        hidden[Cb[0]] = b;
        hidden[Cc[0]] = c;
        int pa = Ca[0], pb = Cb[0], pc = Cc[0];
        G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x==pa||x==pb||x==pc; }), G.end());
    }
    
    // Tail: 2 remaining (original paired logic)
    if(idx + 1 < V.size()){
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
            
            if(sum == 0){ Ca = Sb; Cb = Sa; }
            else if(sum == 2){ Ca = Sa; Cb = Sb; }
            else {
                vector<int> q2(n+1, v_anchor);
                for(int j : Sa) q2[j] = a;
                int r2 = doQuery(q2);
                int ba = r2 - 1;
                if(ba == 1){ Ca = Sa; Cb = Sa; }
                else { Ca = Sb; Cb = Sb; }
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
    
    // Tail: 1 remaining
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