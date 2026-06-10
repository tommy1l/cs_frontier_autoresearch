#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

int main() {
    long long L, R;
    cin >> L >> R;
    
    auto bitlen = [](long long x) {
        int b = 0;
        while (x) { b++; x >>= 1; }
        return b;
    };
    
    int ellL = bitlen(L), ellR = bitlen(R);
    
    int START = newNode();
    int END = newNode();
    
    map<int,int> FREE;
    FREE[0] = END;
    
    function<int(int)> getFree = [&](int k) -> int {
        if (FREE.count(k)) return FREE[k];
        int prev = getFree(k-1);
        int node = newNode();
        FREE[k] = node;
        adj[node-1].push_back({prev, 0});
        adj[node-1].push_back({prev, 1});
        return node;
    };
    
    // Build BL[startIdx..m] where BL[m]=END, backward with FREE-chain reuse.
    auto buildBL = [&](long long suffix, int m, int startIdx) -> int {
        vector<int> BL(m+1);
        BL[m] = END; // = FREE[0]
        bool stillFree = true; // BL[i+1] is FREE[m-i-1]
        for (int i = m-1; i >= startIdx; i--) {
            int bit = (suffix >> (m-1-i)) & 1;
            int freeBelow = -1;
            if (stillFree) {
                // BL[i+1] should equal FREE[m-i-1]
                // ensure that
                freeBelow = getFree(m-i-1);
                // BL[i+1] is already that node by induction
            }
            if (bit == 0 && stillFree) {
                // reuse FREE[m-i]
                BL[i] = getFree(m-i);
                // stillFree stays true
            } else {
                int node = newNode();
                BL[i] = node;
                if (bit == 0) {
                    adj[node-1].push_back({BL[i+1], 0});
                    int fr = getFree(m-i-1);
                    adj[node-1].push_back({fr, 1});
                } else {
                    adj[node-1].push_back({BL[i+1], 1});
                }
                stillFree = false;
            }
        }
        return BL[startIdx];
    };
    
    auto buildBR = [&](long long suffix, int m, int startIdx) -> int {
        vector<int> BR(m+1);
        BR[m] = END;
        bool stillFree = true;
        for (int i = m-1; i >= startIdx; i--) {
            int bit = (suffix >> (m-1-i)) & 1;
            if (bit == 1 && stillFree) {
                BR[i] = getFree(m-i);
            } else {
                int node = newNode();
                BR[i] = node;
                if (bit == 1) {
                    adj[node-1].push_back({BR[i+1], 1});
                    int fr = getFree(m-i-1);
                    adj[node-1].push_back({fr, 0});
                } else {
                    adj[node-1].push_back({BR[i+1], 0});
                }
                stillFree = false;
            }
        }
        return BR[startIdx];
    };
    
    for (int ell = ellL; ell <= ellR; ell++) {
        int T;
        if (ell == 1) {
            T = END;
        } else {
            int m = ell - 1;
            if (ell > ellL && ell < ellR) {
                T = getFree(m);
            } else if (ell == ellL && ell < ellR) {
                long long Lsuffix = L - (1LL << m);
                if (Lsuffix == 0) {
                    T = getFree(m);
                } else {
                    T = buildBL(Lsuffix, m, 0);
                }
            } else if (ell == ellR && ell > ellL) {
                long long Rsuffix = R - (1LL << m);
                if (Rsuffix == ((1LL << m) - 1)) {
                    T = getFree(m);
                } else {
                    T = buildBR(Rsuffix, m, 0);
                }
            } else {
                long long Lsuffix = L - (1LL << m);
                long long Rsuffix = R - (1LL << m);
                
                if (Lsuffix == 0 && Rsuffix == ((1LL << m) - 1)) {
                    T = getFree(m);
                } else {
                    int d = m;
                    for (int i = 0; i < m; i++) {
                        int lb = (Lsuffix >> (m-1-i)) & 1;
                        int rb = (Rsuffix >> (m-1-i)) & 1;
                        if (lb != rb) { d = i; break; }
                    }
                    if (d == m) {
                        if (m == 0) {
                            T = END;
                        } else {
                            vector<int> C(m+1);
                            C[m] = END;
                            for (int i = m-1; i >= 0; i--) {
                                int bit = (Lsuffix >> (m-1-i)) & 1;
                                int node = newNode();
                                C[i] = node;
                                adj[node-1].push_back({C[i+1], bit});
                            }
                            T = C[0];
                        }
                    } else {
                        int blTail = buildBL(Lsuffix, m, d+1);
                        int brTail = buildBR(Rsuffix, m, d+1);
                        int cd;
                        // Check if both tails equal FREE[m-d-1] => C[d] = FREE[m-d]
                        bool canCollapse = false;
                        if (FREE.count(m-d-1)) {
                            int fr = FREE[m-d-1];
                            if (blTail == fr && brTail == fr) canCollapse = true;
                        }
                        if (canCollapse) {
                            cd = getFree(m-d);
                        } else {
                            cd = newNode();
                            adj[cd-1].push_back({blTail, 0});
                            adj[cd-1].push_back({brTail, 1});
                        }
                        vector<int> C(d+1);
                        C[d] = cd;
                        for (int i = d-1; i >= 0; i--) {
                            int bit = (Lsuffix >> (m-1-i)) & 1;
                            int node = newNode();
                            C[i] = node;
                            adj[node-1].push_back({C[i+1], bit});
                        }
                        T = C[0];
                    }
                }
            }
        }
        adj[START-1].push_back({T, 1});
    }
    
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << adj[i].size();
        for (auto& e : adj[i]) {
            cout << " " << e.first << " " << e.second;
        }
        cout << "\n";
    }
    
    return 0;
}