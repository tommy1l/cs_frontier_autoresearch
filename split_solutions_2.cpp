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

static vector<int> setInter(const vector<int>& A, const vector<int>& B){
    set<int> Bs(B.begin(), B.end());
    vector<int> res;
    for(int x : A) if(Bs.count(x)) res.push_back(x);
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
    
    // Process triples
    while(idx + 2 < V.size() && G.size() >= 3){
        int a = V[idx], b = V[idx+1], c = V[idx+2];
        
        // Partition G into P1, P2, P3
        int sz = G.size();
        int s1 = (sz + 2) / 3;
        int rem = sz - s1;
        int s2 = (rem + 1) / 2;
        int s3 = rem - s2;
        
        vector<int> P1(G.begin(), G.begin() + s1);
        vector<int> P2(G.begin() + s1, G.begin() + s1 + s2);
        vector<int> P3(G.begin() + s1 + s2, G.end());
        
        // Round 0 separation
        vector<int> q(n+1, v_anchor);
        for(int p : P1) q[p] = a;
        for(int p : P2) q[p] = b;
        for(int p : P3) q[p] = c;
        int r = doQuery(q);
        int s = r - 1;
        
        vector<int> Ca, Cb, Cc;
        
        if(s == 3){
            Ca = P1; Cb = P2; Cc = P3;
        } else if(s == 0){
            // None in chosen partition
            // a not in P1 -> a in P2 or P3
            // b not in P2 -> b in P1 or P3
            // c not in P3 -> c in P1 or P2
            // Query: place a in P2 only
            vector<int> q2(n+1, v_anchor);
            for(int p : P2) q2[p] = a;
            int r2 = doQuery(q2);
            int ia = r2 - 1; // 1 if a in P2 else a in P3
            if(ia == 1) Ca = P2; else Ca = P3;
            
            // Query: place b in P1 only
            vector<int> q3(n+1, v_anchor);
            for(int p : P1) q3[p] = b;
            int r3 = doQuery(q3);
            int ib = r3 - 1;
            if(ib == 1) Cb = P1; else Cb = P3;
            
            // c is in P1 or P2; deduce by elimination if possible
            // We know a's partition and b's partition. But a,b,c are different positions.
            // If Ca = P3 and Cb = P3: impossible (a != b positions). Actually they could share a partition if partition has size >= 2.
            // Need another query for c
            vector<int> q4(n+1, v_anchor);
            for(int p : P1) q4[p] = c;
            int r4 = doQuery(q4);
            int ic = r4 - 1;
            if(ic == 1) Cc = P1; else Cc = P2;
        } else {
            // s == 1 or s == 2: ambiguous
            // Do localization: place a alone in P1
            vector<int> q2(n+1, v_anchor);
            for(int p : P1) q2[p] = a;
            int r2 = doQuery(q2);
            int ia_p1 = r2 - 1; // 1 if a in P1
            
            // Place b alone in P2
            vector<int> q3(n+1, v_anchor);
            for(int p : P2) q3[p] = b;
            int r3 = doQuery(q3);
            int ib_p2 = r3 - 1;
            
            // For a: if ia_p1=1 -> P1, else need to distinguish P2/P3
            // For b: if ib_p2=1 -> P2, else need P1/P3
            // For c: need to determine
            // From original s: i_a + i_b + i_c = s where i_x = 1 if x in its chosen part (a in P1, b in P2, c in P3)
            // We know ia_p1 = i_a (whether a in P1), ib_p2 = i_b
            // So i_c = s - ia_p1 - ib_p2
            int ic_p3 = s - ia_p1 - ib_p2;
            
            // Now we know for each value whether it's in its "chosen" part
            // a: in P1 iff ia_p1=1; else in P2 or P3
            // b: in P2 iff ib_p2=1; else in P1 or P3
            // c: in P3 iff ic_p3=1; else in P1 or P2
            
            // If all are in chosen, that's s=3 case (impossible here)
            // We have at most one more query budget. Let's use it strategically.
            
            // Determine unknowns
            bool a_known = (ia_p1 == 1);
            bool b_known = (ib_p2 == 1);
            bool c_known = (ic_p3 == 1);
            
            if(a_known) Ca = P1;
            if(b_known) Cb = P2;
            if(c_known) Cc = P3;
            
            int unknown_count = (a_known?0:1) + (b_known?0:1) + (c_known?0:1);
            
            if(unknown_count == 1){
                // The one unknown is determined by elimination... actually not really, we know it's not in its chosen, so it's in one of two others.
                // But all three positions are different. If two are known (e.g., a in P1, b in P2), c is not in P3 so c in P1 or P2.
                // Still ambiguous between two partitions. Need another query.
                if(!a_known){
                    vector<int> q4(n+1, v_anchor);
                    for(int p : P2) q4[p] = a;
                    int r4 = doQuery(q4);
                    if(r4 - 1 == 1) Ca = P2; else Ca = P3;
                } else if(!b_known){
                    vector<int> q4(n+1, v_anchor);
                    for(int p : P1) q4[p] = b;
                    int r4 = doQuery(q4);
                    if(r4 - 1 == 1) Cb = P1; else Cb = P3;
                } else {
                    vector<int> q4(n+1, v_anchor);
                    for(int p : P1) q4[p] = c;
                    int r4 = doQuery(q4);
                    if(r4 - 1 == 1) Cc = P1; else Cc = P2;
                }
            } else if(unknown_count == 2){
                // Two unknowns. We've used 3 queries already (initial + 2 localization). Budget says cap at 3.
                // But we still need to resolve. Use Ca/Cb/Cc as union of possible partitions.
                if(!a_known){
                    // a in P2 or P3
                    Ca = P2; Ca.insert(Ca.end(), P3.begin(), P3.end());
                }
                if(!b_known){
                    Cb = P1; Cb.insert(Cb.end(), P3.begin(), P3.end());
                }
                if(!c_known){
                    Cc = P1; Cc.insert(Cc.end(), P2.begin(), P2.end());
                }
            } else if(unknown_count == 3){
                // s = 0 case actually, but s != 0 here. So unknown_count <= 2 when s>=1
                // Defensive
                Ca = G; Cb = G; Cc = G;
            }
        }
        
        // Now binary search per value, round-robin
        auto refine = [&](vector<int>& C, int val){
            if(C.size() <= 1) return;
            int sz = C.size();
            int half = (sz + 1) / 2;
            vector<int> S(C.begin(), C.begin() + half);
            vector<int> q(n+1, v_anchor);
            for(int p : S) q[p] = val;
            int r = doQuery(q);
            int ind = r - 1;
            if(ind == 1) C = S;
            else C = vector<int>(C.begin() + half, C.end());
        };
        
        while(Ca.size() > 1 || Cb.size() > 1 || Cc.size() > 1){
            if(Ca.size() > 1) refine(Ca, a);
            if(Cb.size() > 1) refine(Cb, b);
            if(Cc.size() > 1) refine(Cc, c);
        }
        
        hidden[Ca[0]] = a;
        hidden[Cb[0]] = b;
        hidden[Cc[0]] = c;
        int pa = Ca[0], pb = Cb[0], pc = Cc[0];
        G.erase(remove_if(G.begin(), G.end(), [&](int x){ return x == pa || x == pb || x == pc; }), G.end());
        idx += 3;
    }
    
    // Leftover: paired
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