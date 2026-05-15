#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <random>
#include <bits/stdc++.h>

using ll = long long;
using namespace std;

// --- Problem Constraints ---
const long long MAX_MASS = 20LL * 1000 * 1000; // 20 kg in mg
const long long MAX_VOLUME = 25LL * 1000 * 1000; // 25 L in µL
const int BEST_SOLUTION_RUNTIME_MS = 1000; // Run randomized search for 3 seconds

// Structure to hold item properties for sorting
struct Item {
    std::string name;
    long long q, v, m, l;
    double density;
};

// --- Simple JSON Parser for This Specific Problem ---
// NOTE: This is NOT a general-purpose JSON parser. It's fragile and expects
// the exact format: {"key":[q,v,m,l], ...} with 12 items.
std::map<std::string, std::vector<long long>> parseInput() {
    std::map<std::string, std::vector<long long>> itemsData;
    char ch;

    cin >> ch; // Read '{'
    for (int i = 0; i < 12; ++i) {
        // Read key
        cin >> ch; // Read '"'
        std::string key;
        std::getline(cin, key, '"');
        
        cin >> ch; // Read ':'
        cin >> ch; // Read '['
        
        long long q, v, m, l;
        cin >> q >> ch >> v >> ch >> m >> ch >> l;
        
        cin >> ch; // Read ']'
        
        itemsData[key] = {q, v, m, l};
        
        if (i < 11) {
            cin >> ch; // Read ','
        }
    }
    cin >> ch; // Read '}'

    return itemsData;
}

// --- Solution Value Calculator ---
long long calculateSolutionValue(const std::map<std::string, std::vector<long long>>& itemsData, const std::map<std::string, long long>& solutionCounts) {
    long long totalValue = 0;
    for (const auto& pair : solutionCounts) {
        totalValue += pair.second * itemsData.at(pair.first)[1]; // count * value
    }
    return totalValue;
}


map <string, ll> g_bestSolutionCounts;

long long generateBestSolution(const std::map<std::string, std::vector<long long>>& itemsData) {
    using namespace std;
    struct Item { string name; ll q,v,m,l; };
    vector<Item> items; items.reserve(itemsData.size());
    for (auto& kv: itemsData) items.push_back({kv.first, kv.second[0], kv.second[1], kv.second[2], kv.second[3]});
    int n = (int)items.size();
    mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
    auto nowms = [](){ return (long long)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count(); };
    long long start = nowms();
    long long tend = start + BEST_SOLUTION_RUNTIME_MS - 5;
    long long gen_end = start + 500;
    struct Cand { long long val; vector<ll> cnt; };
    struct MinCmp { bool operator()(Cand const& a, Cand const& b) const { return a.val > b.val; } };
    const int TOPK = 1000;
    priority_queue<Cand, vector<Cand>, MinCmp> pq;
    vector<int> idx(n); iota(idx.begin(), idx.end(), 0);
    auto make_cand = [&](vector<ll>& cnt, long long& val){
        cnt.assign(n,0); val=0;
        vector<int> perm = idx; shuffle(perm.begin(), perm.end(), rng);
        ll usedM=0, usedL=0;
        int a=perm[0], b=perm[1];
        ll maxA = min({items[a].q, (MAX_MASS-usedM)/items[a].m, (MAX_VOLUME-usedL)/items[a].l});
        ll takeA = maxA>0? uniform_int_distribution<long long>(0,maxA)(rng) : 0;
        cnt[a]=takeA; usedM+=takeA*items[a].m; usedL+=takeA*items[a].l; val+=takeA*items[a].v;
        ll maxB = min({items[b].q, (MAX_MASS-usedM)/items[b].m, (MAX_VOLUME-usedL)/items[b].l});
        ll takeB = maxB>0? uniform_int_distribution<long long>(0,maxB)(rng) : 0;
        cnt[b]=takeB; usedM+=takeB*items[b].m; usedL+=takeB*items[b].l; val+=takeB*items[b].v;
        long double am = 1.0L/(long double)max< ll >(1, MAX_MASS-usedM);
        long double av = 1.0L/(long double)max< ll >(1, MAX_VOLUME-usedL);
        vector<pair<long double,int>> ord; ord.reserve(n-2);
        for (int k=2;k<n;++k){ int i=perm[k]; long double den = am*items[i].m + av*items[i].l; long double sc = (den>0)? ((long double)items[i].v/den) : (long double)items[i].v; ord.push_back({sc,i}); }
        sort(ord.begin(), ord.end(), [&](auto& x, auto& y){ if (x.first!=y.first) return x.first>y.first; return items[x.second].v>items[y.second].v; });
        for (auto& p: ord){
            int i=p.second;
            ll capM = MAX_MASS - usedM, capL = MAX_VOLUME - usedL;
            if (capM<=0 || capL<=0) break;
            ll take = min({items[i].q, capM/items[i].m, capL/items[i].l});
            if (take<=0) continue;
            cnt[i]=take; usedM+=take*items[i].m; usedL+=take*items[i].l; val+=take*items[i].v;
        }
    };
    while (nowms() < gen_end) {
        vector<ll> cnt; long long val; make_cand(cnt,val);
        if ((int)pq.size() < TOPK) pq.push({val, move(cnt)});
        else if (val > pq.top().val) { pq.pop(); pq.push({val, move(cnt)}); }
    }
    vector<Cand> pool; pool.reserve(pq.size());
    while (!pq.empty()) { pool.push_back(move(const_cast<Cand&>(pq.top()))); pq.pop(); }
    sort(pool.begin(), pool.end(), [](const Cand& a, const Cand& b){ return a.val>b.val; });
    auto two_opt = [&](int a, int b, ll Mrem, ll Vrem, ll& outA, ll& outB, long long& outVal){
        ll maxA = min({items[a].q, Mrem/items[a].m, Vrem/items[a].l});
        ll maxB = min({items[b].q, Mrem/items[b].m, Vrem/items[b].l});
        outA=0; outB=0; outVal=0;
        if (maxA<=maxB){
            for (ll k=0;k<=maxA;++k){
                ll m2=Mrem - k*items[a].m, l2=Vrem - k*items[a].l; if (m2<0||l2<0) break;
                ll kb=min({items[b].q, m2/items[b].m, l2/items[b].l});
                long long val=k*items[a].v + kb*items[b].v;
                if (val>outVal){ outVal=val; outA=k; outB=kb; }
            }
        } else {
            for (ll k=0;k<=maxB;++k){
                ll m2=Mrem - k*items[b].m, l2=Vrem - k*items[b].l; if (m2<0||l2<0) break;
                ll ka=min({items[a].q, m2/items[a].m, l2/items[a].l});
                long long val=ka*items[a].v + k*items[b].v;
                if (val>outVal){ outVal=val; outA=ka; outB=k; }
            }
        }
    };
    auto triple_opt = [&](int a, int b, int c, ll Mrem, ll Vrem, ll& outA, ll& outB, ll& outC, long long& outVal){
        outA=0; outB=0; outC=0; outVal=0;
        ll maxA = min({items[a].q, Mrem/items[a].m, Vrem/items[a].l});
        for (ll ka=0; ka<=maxA; ++ka){
            ll m1 = Mrem - ka*items[a].m, l1 = Vrem - ka*items[a].l;
            if (m1<0 || l1<0) break;
            ll maxB = min({items[b].q, m1/items[b].m, l1/items[b].l});
            ll maxC0 = min({items[c].q, m1/items[c].m, l1/items[c].l});
            if (maxB<=maxC0){
                for (ll kb=0; kb<=maxB; ++kb){
                    ll m2=m1 - kb*items[b].m, l2=l1 - kb*items[b].l;
                    if (m2<0 || l2<0) break;
                    ll kc=min({items[c].q, m2/items[c].m, l2/items[c].l});
                    long long val=ka*items[a].v + kb*items[b].v + kc*items[c].v;
                    if (val>outVal){ outVal=val; outA=ka; outB=kb; outC=kc; }
                }
            } else {
                for (ll kc=0; kc<=maxC0; ++kc){
                    ll m2=m1 - kc*items[c].m, l2=l1 - kc*items[c].l;
                    if (m2<0 || l2<0) break;
                    ll kb=min({items[b].q, m2/items[b].m, l2/items[b].l});
                    long long val=ka*items[a].v + kb*items[b].v + kc*items[c].v;
                    if (val>outVal){ outVal=val; outA=ka; outB=kb; outC=kc; }
                }
            }
        }
    };
    long long bestVal = 0; vector<ll> bestCnt(n,0);
    uniform_int_distribution<int> dist(0,n-1);
    for (size_t p=0; p<pool.size() && nowms()<tend; ++p){
        vector<ll> cur = pool[p].cnt;
        ll usedM=0, usedL=0; long long curVal=0;
        for (int i=0;i<n;++i){ usedM+=cur[i]*items[i].m; usedL+=cur[i]*items[i].l; curVal+=cur[i]*items[i].v; }
        if (curVal>bestVal){ bestVal=curVal; bestCnt=cur; }
        size_t pairs=(size_t)n*(n-1)/2, stall=0;
        while (nowms()<tend && stall<pairs*4){
            int i=dist(rng), j=dist(rng); if (i==j){ ++stall; continue; } if (j<i) swap(i,j);
            ll pairM = cur[i]*items[i].m + cur[j]*items[j].m;
            ll pairL = cur[i]*items[i].l + cur[j]*items[j].l;
            ll Mrem = MAX_MASS - (usedM - pairM);
            ll Vrem = MAX_VOLUME - (usedL - pairL);
            if (Mrem<=0 || Vrem<=0){ ++stall; continue; }
            ll ai=0,bj=0; long long bestPair=0;
            two_opt(i,j,Mrem,Vrem,ai,bj,bestPair);
            long long curPair = cur[i]*items[i].v + cur[j]*items[j].v;
            if (bestPair>curPair){
                usedM = usedM - pairM + ai*items[i].m + bj*items[j].m;
                usedL = usedL - pairL + ai*items[i].l + bj*items[j].l;
                curVal = curVal - curPair + bestPair;
                cur[i]=ai; cur[j]=bj;
                stall=0;
                if (curVal>bestVal){ bestVal=curVal; bestCnt=cur; }
            } else ++stall;
        }
        // Triple optimization pass: enumerate all triples once and lock in improvements
        for (int i=0; i<n && nowms()<tend; ++i){
            for (int j=i+1; j<n && nowms()<tend; ++j){
                for (int k=j+1; k<n && nowms()<tend; ++k){
                    ll tripM = cur[i]*items[i].m + cur[j]*items[j].m + cur[k]*items[k].m;
                    ll tripL = cur[i]*items[i].l + cur[j]*items[j].l + cur[k]*items[k].l;
                    ll Mrem = MAX_MASS - (usedM - tripM);
                    ll Vrem = MAX_VOLUME - (usedL - tripL);
                    if (Mrem<=0 || Vrem<=0) continue;
                    // Skip if smallest q*m exceeds threshold (avoid cubic blowup on huge q)
                    ll maxI = min({items[i].q, Mrem/items[i].m, Vrem/items[i].l});
                    if (maxI > 300) continue;
                    ll ai2=0, bj2=0, ck2=0; long long bestTrip=0;
                    triple_opt(i,j,k,Mrem,Vrem,ai2,bj2,ck2,bestTrip);
                    long long curTrip = cur[i]*items[i].v + cur[j]*items[j].v + cur[k]*items[k].v;
                    if (bestTrip>curTrip){
                        usedM = usedM - tripM + ai2*items[i].m + bj2*items[j].m + ck2*items[k].m;
                        usedL = usedL - tripL + ai2*items[i].l + bj2*items[j].l + ck2*items[k].l;
                        curVal = curVal - curTrip + bestTrip;
                        cur[i]=ai2; cur[j]=bj2; cur[k]=ck2;
                        if (curVal>bestVal){ bestVal=curVal; bestCnt=cur; }
                    }
                }
            }
        }
    }
    if (bestVal==0 && !pool.empty()){ bestVal=pool[0].val; bestCnt=pool[0].cnt; }
    g_bestSolutionCounts.clear();
    for (int i=0;i<n;++i) g_bestSolutionCounts[items[i].name]=bestCnt[i];
    return bestVal;
}

// --- Main Execution ---
int main(int argc, char* argv[]) {
    auto itemsData = parseInput();

    long long bestValue = generateBestSolution(itemsData);

    std::cout << "{";
    bool first = true;
    for (const auto& item : itemsData) {
        if (!first) std::cout << ",";
        std::cout << "\n  \"" << item.first << "\": " << g_bestSolutionCounts[item.first];
        first = false;
    }
    std::cout << "\n}\n";
    return 0;
}