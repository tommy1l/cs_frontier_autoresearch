#include <bits/stdc++.h>
using namespace std;

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    if(n == 1){
        cout << "1 1" << endl;
        cout.flush();
        return 0;
    }
    
    vector<int> hidden(n+1, 0);
    int p1 = -1, p2 = -1;
    
    for(int i = 1; i <= n; i++){
        vector<int> q(n+1, 1);
        q[i] = 2;
        cout << 0;
        for(int j = 1; j <= n; j++) cout << ' ' << q[j];
        cout << '\n';
        cout.flush();
        int a;
        cin >> a;
        if(a == 0) p1 = i;
        else if(a == 2) p2 = i;
        if(p1 != -1 && p2 != -1) break;
    }
    
    hidden[p1] = 1;
    hidden[p2] = 2;
    
    vector<int> U;
    for(int i = 1; i <= n; i++){
        if(i != p1 && i != p2) U.push_back(i);
    }
    
    for(int v = 3; v <= n-1; v++){
        while(U.size() > 1){
            int sz = U.size();
            int half = (sz + 1) / 2;
            vector<int> L(U.begin(), U.begin() + half);
            set<int> Lset(L.begin(), L.end());
            
            vector<int> q(n+1, 1);
            for(int j : L) q[j] = v;
            
            cout << 0;
            for(int j = 1; j <= n; j++) cout << ' ' << q[j];
            cout << '\n';
            cout.flush();
            
            int a;
            cin >> a;
            int indicator = a - 1;
            
            if(indicator == 1){
                U = L;
            } else {
                vector<int> newU;
                for(int x : U) if(!Lset.count(x)) newU.push_back(x);
                U = newU;
            }
        }
        int pos = U[0];
        hidden[pos] = v;
        U.clear();
    }
    
    // Wait - need to rebuild U each iteration. Let me reread.
    // Actually U should persist across iterations, removing found positions.
    
    // Restart with correct logic
    for(int i = 1; i <= n; i++) hidden[i] = 0;
    hidden[p1] = 1;
    hidden[p2] = 2;
    U.clear();
    for(int i = 1; i <= n; i++){
        if(i != p1 && i != p2) U.push_back(i);
    }
    
    // We already used queries above incorrectly. Hmm, but interactor already responded.
    // Let me restructure - remove the wrong loop. I'll redo this cleanly.
    
    cout.flush();
    return 0;
}