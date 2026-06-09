#include <bits/stdc++.h>
using namespace std;

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    if(n == 1){
        cout << "1 1" << '\n';
        cout.flush();
        return 0;
    }
    
    vector<int> hidden(n+1, 0);
    int p_anchor = -1, v_anchor = -1;
    
    for(int k = 1; k <= n; k++){
        int i;
        if(k % 2 == 1) i = (k + 1) / 2;
        else i = n + 1 - (k / 2);
        
        vector<int> q(n+1, 1);
        q[i] = 2;
        cout << 0;
        for(int j = 1; j <= n; j++) cout << ' ' << q[j];
        cout << '\n';
        cout.flush();
        int a;
        cin >> a;
        if(a == 0){
            p_anchor = i;
            v_anchor = 1;
            break;
        } else if(a == 2){
            p_anchor = i;
            v_anchor = 2;
            break;
        }
    }
    
    hidden[p_anchor] = v_anchor;
    
    vector<int> G;
    for(int i = 1; i <= n; i++){
        if(i != p_anchor) G.push_back(i);
    }
    
    for(int v = 1; v <= n-1; v++){
        if(v == v_anchor) continue;
        
        vector<int> C = G;
        
        while(C.size() > 1){
            int sz = C.size();
            int half = (sz + 1) / 2;
            vector<int> L(C.begin(), C.begin() + half);
            unordered_set<int> Lset(L.begin(), L.end());
            
            vector<int> q(n+1, v_anchor);
            for(int j : L) q[j] = v;
            
            cout << 0;
            for(int j = 1; j <= n; j++) cout << ' ' << q[j];
            cout << '\n';
            cout.flush();
            
            int a;
            cin >> a;
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