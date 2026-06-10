#include <bits/stdc++.h>
using namespace std;

int n = 0;
vector<vector<pair<int,int>>> adj;

int newNode() {
    n++;
    adj.push_back({});
    return n;
}

int END_NODE;
int START_NODE;
map<string, int> profMemo;

struct Trip { int m; long long lo, hi; };

string serialize(vector<Trip>& P) {
    sort(P.begin(), P.end(), [](const Trip& a, const Trip& b){ return a.m < b.m; });
    string s;
    for (auto& t : P) {
        s += to_string(t.m); s += '-';
        s += to_string(t.lo); s += '-';
        s += to_string(t.hi); s += '-';
    }
    return s;
}

int buildNode(vector<Trip> profile, long long v) {
    if (profile.empty()) return -1;
    string key = serialize(profile);
    auto it = profMemo.find(key);
    if (it != profMemo.end()) return it->second;
    int node = newNode();
    profMemo[key] = node;
    
    for (int b = 0; b < 2; b++) {
        vector<Trip> child;
        bool stopHere = false;
        for (auto& t : profile) {
            long long half = 1LL << (t.m - 1);
            long long clo, chi;
            if (b == 0) {
                clo = max(0LL, t.lo);
                chi = min(half - 1, t.hi);
            } else {
                clo = max(0LL, t.lo - half);
                chi = min(half - 1, t.hi - half);
            }
            if (clo > chi) continue;
            if (t.m == 1) {
                if (clo == 0 && chi == 0) stopHere = true;
            } else {
                child.push_back({t.m - 1, clo, chi});
            }
        }
        if (stopHere) adj[node-1].push_back({END_NODE, b});
        if (!child.empty()) {
            int cid = buildNode(child, 2*v + b);
            if (cid != -1) adj[node-1].push_back({cid, b});
        }
    }
    return node;
}

void buildForward(long long L, long long R) {
    START_NODE = newNode();
    END_NODE = newNode();
    
    auto bitlen = [](long long x) {
        int b = 0;
        while (x) { b++; x >>= 1; }
        return b;
    };
    
    int ellL = bitlen(L), ellR = bitlen(R);
    
    vector<Trip> P1;
    for (int m = ellL - 1; m <= ellR - 1; m++) {
        if (m < 1) continue;
        long long pw = 1LL << m;
        long long lo = max(0LL, L - pw);
        long long hi = min(pw - 1, R - pw);
        if (lo <= hi) P1.push_back({m, lo, hi});
    }
    
    if (L == 1) {
        adj[START_NODE-1].push_back({END_NODE, 1});
    }
    if (!P1.empty()) {
        int c = buildNode(P1, 1);
        if (c != -1) adj[START_NODE-1].push_back({c, 1});
    }
}

void printGraph(int nn, vector<vector<pair<int,int>>>& ad, int startId, int endId) {
    vector<int> remap(nn+1, 0);
    remap[startId] = 1;
    int nxt = 2;
    for (int i = 1; i <= nn; i++) {
        if (i == startId) continue;
        remap[i] = nxt++;
    }
    
    cout << nn << "\n";
    vector<vector<pair<int,int>>> out(nn+1);
    for (int i = 1; i <= nn; i++) {
        for (auto& e : ad[i-1]) {
            out[remap[i]].push_back({remap[e.first], e.second});
        }
    }
    for (int i = 1; i <= nn; i++) {
        cout << out[i].size();
        for (auto& e : out[i]) {
            cout << " " << e.first << " " << e.second;
        }
        cout << "\n";
    }
}

int main() {
    long long L, R;
    cin >> L >> R;
    
    buildForward(L, R);
    
    int nForward = n;
    vector<vector<pair<int,int>>> forwardAdj = adj;
    int fwdStart = START_NODE, fwdEnd = END_NODE;
    
    vector<array<vector<int>,2>> revAdj(nForward+1);
    for (int u = 1; u <= nForward; u++) {
        for (auto& e : forwardAdj[u-1]) {
            revAdj[e.first][e.second].push_back(u);
        }
    }
    
    auto encodeSet = [](vector<int>& s) {
        string r;
        for (int x : s) { r += to_string(x); r += ','; }
        return r;
    };
    
    unordered_map<string,int> subsetId;
    vector<vector<int>> subsets;
    
    int runningNonAcceptCount = 0;
    bool anyAccept = false;
    
    vector<int> initSet = {fwdEnd};
    sort(initSet.begin(), initSet.end());
    string initKey = encodeSet(initSet);
    subsetId[initKey] = 0;
    subsets.push_back(initSet);
    if (binary_search(initSet.begin(), initSet.end(), fwdStart)) {
        anyAccept = true;
    } else {
        runningNonAcceptCount++;
    }
    
    struct DRTrans { int from, w, to; };
    vector<DRTrans> drTrans;
    
    queue<int> bfs;
    bfs.push(0);
    bool bailout = false;
    
    while (!bfs.empty() && !bailout) {
        int sid = bfs.front(); bfs.pop();
        vector<int> S = subsets[sid];
        for (int w = 0; w < 2; w++) {
            set<int> Tset;
            for (int u : S) {
                for (int v : revAdj[u][w]) Tset.insert(v);
            }
            if (Tset.empty()) continue;
            vector<int> T(Tset.begin(), Tset.end());
            string k = encodeSet(T);
            auto it = subsetId.find(k);
            int tid;
            if (it == subsetId.end()) {
                tid = subsets.size();
                subsetId[k] = tid;
                subsets.push_back(T);
                bool isAcc = binary_search(T.begin(), T.end(), fwdStart);
                if (isAcc) anyAccept = true;
                else runningNonAcceptCount++;
                
                int currentReverseSize = runningNonAcceptCount + (anyAccept ? 1 : 0);
                if (currentReverseSize > 99 || (int)subsets.size() > 4096) {
                    bailout = true;
                    break;
                }
                bfs.push(tid);
            } else {
                tid = it->second;
            }
            drTrans.push_back({sid, w, tid});
        }
    }
    
    if (bailout || !anyAccept) {
        printGraph(nForward, forwardAdj, fwdStart, fwdEnd);
        return 0;
    }
    
    int numSubsets = subsets.size();
    vector<bool> isAccept(numSubsets, false);
    for (int i = 0; i < numSubsets; i++) {
        if (binary_search(subsets[i].begin(), subsets[i].end(), fwdStart)) {
            isAccept[i] = true;
        }
    }
    
    vector<int> nfId(numSubsets, 0);
    int mergedStart = 1;
    int nfCount = 1;
    for (int i = 0; i < numSubsets; i++) {
        if (isAccept[i]) {
            nfId[i] = mergedStart;
        } else {
            nfCount++;
            nfId[i] = nfCount;
        }
    }
    
    int nReverse = nfCount;
    
    if (nForward <= nReverse) {
        printGraph(nForward, forwardAdj, fwdStart, fwdEnd);
        return 0;
    }
    
    vector<vector<pair<int,int>>> nfAdj(nReverse);
    for (auto& tr : drTrans) {
        int from = nfId[tr.to];
        int to = nfId[tr.from];
        nfAdj[from-1].push_back({to, tr.w});
    }
    
    int nfEnd = nfId[0];
    int nfStart = mergedStart;
    
    n = nReverse;
    adj = nfAdj;
    printGraph(nReverse, nfAdj, nfStart, nfEnd);
    
    return 0;
}