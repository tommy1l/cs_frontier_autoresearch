#include <bits/stdc++.h>
using namespace std;

// Approach 2: canonical interval decomposition.
// [L, R] is decomposed into intervals [a*2^k, (a+1)*2^k - 1].
// Each interval is a prefix (a in binary) followed by k arbitrary bits.
// Build a trie of prefixes, with each leaf connecting to a shared free chain.

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    long long L, R;
    cin >> L >> R;

    // Generate canonical intervals.
    // Each interval: (prefix value, prefix_len, k).
    // The prefix is a binary string of length prefix_len, then k free bits.
    vector<tuple<long long, int, int>> intervals; // (prefix_val, prefix_len, k)
    long long cur = L;
    while (cur <= R) {
        int k = 0;
        // Find largest k such that cur is divisible by 2^k AND cur + 2^k - 1 <= R.
        while (k < 30 && (cur & ((1LL << (k+1)) - 1)) == 0 && cur + (1LL << (k+1)) - 1 <= R) {
            k++;
        }
        long long sz = 1LL << k;
        long long pref_val = cur >> k;
        int pref_len = 0;
        for (long long x = pref_val; x; x >>= 1) pref_len++;
        intervals.push_back({pref_val, pref_len, k});
        cur += sz;
    }

    // Build trie of prefixes. trie_node[node_id] = (edge weight 0 child, edge weight 1 child).
    // Also: a leaf points to free_k for its k.
    // We share the free chain across all intervals.

    // Free chain: free_node[k] = node id for "k free bits remaining". free_node[0] = end.
    int max_k = 0;
    for (auto& [p, pl, k] : intervals) max_k = max(max_k, k);

    // Node id assignment:
    //   0 = start
    //   1..(max_k+1) = free chain (1 = free_0 = end, ..., max_k+1 = free_{max_k})
    //   Then trie nodes.

    vector<vector<pair<int,int>>> edges;
    int start = 0;
    edges.push_back({}); // node 0 = start

    // Free chain: free_k_id[k] = node for "k free bits remaining".
    vector<int> free_id(max_k + 1);
    free_id[0] = (int)edges.size();
    edges.push_back({}); // end node
    for (int k = 1; k <= max_k; k++) {
        free_id[k] = (int)edges.size();
        edges.push_back({});
        edges[free_id[k]].push_back({free_id[k-1], 0});
        edges[free_id[k]].push_back({free_id[k-1], 1});
    }

    // Build trie. trie root corresponds to "" (empty prefix), reached from start via... hmm.
    // Actually each prefix p (length pl) starts with 1 (no leading zeros for any number).
    // The trie's root is "" but the start node has 1 outgoing edge weight 1 to "1" (next node).
    // So we build the trie rooted at "1" or use the start node as root with weight 1 children.

    // Build prefix trie. Use a map (prefix_val, prefix_len) -> node id.
    // Note prefixes can have different lengths, so we need a more careful representation.

    // Use binary trie: each node has children for bit 0 and bit 1.
    // The "" trie node = node id for start? Start has outgoing edge weight 1 only.
    // So start = "" trie node, with 1 edge weight 1 → "1" trie node.

    // For each prefix p, we add a path in the trie from start to the prefix node.
    // The prefix node then has 2 edges to free_{k_i - 1} (with weights 0, 1), OR if k_i = 0, prefix node = end.

    // Build trie iteratively.
    map<pair<long long, int>, int> trie_id; // (val, len) -> node id
    trie_id[{0, 0}] = start; // empty prefix = start

    auto get_or_create = [&](long long val, int len) -> int {
        auto it = trie_id.find({val, len});
        if (it != trie_id.end()) return it->second;
        int id = (int)edges.size();
        trie_id[{val, len}] = id;
        edges.push_back({});
        return id;
    };

    for (auto& [pv, pl, k] : intervals) {
        // Walk down the trie from root, creating nodes as needed.
        int cur_node = start;
        for (int bit_pos = pl - 1; bit_pos >= 0; bit_pos--) {
            int bit = (pv >> bit_pos) & 1;
            long long parent_val = pv >> (bit_pos + 1);
            int parent_len = pl - 1 - bit_pos;
            long long child_val = pv >> bit_pos;
            int child_len = pl - bit_pos;
            int child_node = get_or_create(child_val, child_len);
            // Check if edge already exists
            bool found = false;
            for (auto& e : edges[cur_node]) {
                if (e.first == child_node && e.second == bit) { found = true; break; }
            }
            if (!found) edges[cur_node].push_back({child_node, bit});
            cur_node = child_node;
        }
        // cur_node = leaf for this prefix. Connect to free_{k}.
        if (k == 0) {
            // The prefix IS the number. The leaf should be end. But we've created a separate leaf node.
            // We need to merge this leaf with end. To simplify, redirect leaf's role to end.
            // Easier: don't add the last bit to a new leaf; instead, point to end via the last bit.
            // Refactor: redo this case.
            // For simplicity, if k=0, the leaf is end. Replace cur_node with end (free_id[0]).
            // But we already created cur_node. To merge, replace all references.
            // Simpler: redo this case by stopping one bit early and adding the last bit as edge to end.

            // Backtrack: remove last edge and replace target with end.
            // Actually, this is messy. Let's handle differently above.
            // (Will handle below.)
        }
    }

    // Hmm, the above is getting complex. Let me redo cleanly using a recursive approach.

    // Reset.
    edges.clear();
    trie_id.clear();

    // Allocate start = 0.
    start = (int)edges.size();
    edges.push_back({});
    // Allocate end = 1.
    int end_node = (int)edges.size();
    edges.push_back({});
    // Free chain: free_id[k] for k = 0..max_k. free_id[0] = end.
    free_id.assign(max_k + 1, -1);
    free_id[0] = end_node;
    for (int k = 1; k <= max_k; k++) {
        free_id[k] = (int)edges.size();
        edges.push_back({});
        edges[free_id[k]].push_back({free_id[k-1], 0});
        edges[free_id[k]].push_back({free_id[k-1], 1});
    }

    trie_id[{0, 0}] = start;

    auto get_create = [&](long long val, int len) -> int {
        auto it = trie_id.find({val, len});
        if (it != trie_id.end()) return it->second;
        int id = (int)edges.size();
        trie_id[{val, len}] = id;
        edges.push_back({});
        return id;
    };

    for (auto& [pv, pl, k] : intervals) {
        int cur_node = start;
        for (int bit_pos = pl - 1; bit_pos > 0; bit_pos--) {
            int bit = (pv >> bit_pos) & 1;
            long long child_val = pv >> bit_pos;
            int child_len = pl - bit_pos;
            int child_node = get_create(child_val, child_len);
            bool found = false;
            for (auto& e : edges[cur_node]) {
                if (e.first == child_node && e.second == bit) { found = true; break; }
            }
            if (!found) edges[cur_node].push_back({child_node, bit});
            cur_node = child_node;
        }
        // Last bit of prefix (bit_pos = 0). After this bit, we should be at free_{k} (or end if k=0).
        int last_bit = pv & 1;
        int target = free_id[k];
        bool found = false;
        for (auto& e : edges[cur_node]) {
            if (e.first == target && e.second == last_bit) { found = true; break; }
        }
        if (!found) edges[cur_node].push_back({target, last_bit});
    }

    int n = (int)edges.size();
    cout << n << "\n";
    for (int i = 0; i < n; i++) {
        cout << edges[i].size();
        for (auto& e : edges[i]) {
            cout << " " << (e.first + 1) << " " << e.second;
        }
        cout << "\n";
    }
    return 0;
}
