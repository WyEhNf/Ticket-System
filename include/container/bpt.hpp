#pragma once
#include "list.hpp"
#include "map.hpp"
#include "../memoryriver/memoryriver.hpp"
#include "vector.hpp"

using namespace std;
namespace sjtu {
template <typename IndexType, typename ValueType, int ORDER = 32>
class BPlusTree {
    public:
    struct Key {
        IndexType index;
        ValueType value;
        bool operator==(const Key& o) const {
            return index == o.index &&
                   value == o.value;
        }
        bool operator<(const Key& o) const {
            if (!(index == o.index))
                return index< o.index;
            return value < o.value;
        }
    };

   private:
    struct Node {
        bool is_leaf = false;
        int key_cnt = 0;
        int parent = -1;
        int next = -1;
        Key keys[ORDER];
        int child[ORDER + 1];
    };
    MemoryRiver<Node, 2> river;
    struct CacheBlock {
        Node node;
        bool dirty = false;
        int pos;
    };

    static constexpr int CACHE_CAPACITY = 200;
    list<CacheBlock> cache_list;
    map<int, typename list<CacheBlock>::iterator> cache_map;

    Node node(int pos);
    void write_node(const Node& x, int pos);
    Node add_to_cache(const Node& x, int pos);
    void flush_all();
    int min_leaf_keys() const;
    int min_internal_keys() const;
    int root();
    void set_root(int r);
    int new_node(const Node& x);
    int find_leaf(const Key& key);
    void insert_in_leaf(int u, const Key& key);
    void split_leaf(int u);
    void insert_in_parent(int u, const Key& key, int v);
    void split_internal(int u);
    void fix_leaf(int u);
    void merge_leaf(int l, int r, int sep);
    void fix_internal(int u);
    void merge_internal(int l, int r, int sep);
    
   public:
    BPlusTree(string fn = "bpt.db");
    ~BPlusTree();
    void insert(const IndexType& idx, const ValueType& val);
    void erase(const IndexType& idx, const ValueType& val);
    vector<Key> find(const IndexType& idx);
    void clean_up();
};


template <typename IndexType, typename ValueType, int ORDER>
typename BPlusTree<IndexType, ValueType, ORDER>::Node
BPlusTree<IndexType, ValueType, ORDER>::node(int pos) {
    auto map_it = cache_map.find(pos);
    if (map_it != cache_map.end()) {
        auto old_list_it = map_it->second;
        Node current_node = old_list_it->node;
        bool current_dirty = old_list_it->dirty;
        cache_list.erase(old_list_it);
        cache_list.push_front({current_node, current_dirty, pos});
        cache_map[pos] = cache_list.begin();
        return current_node;
    }
    Node x;
    river.read(x, pos);
    if (cache_list.size() >= CACHE_CAPACITY) {
        CacheBlock& victim = cache_list.back();
        if (victim.dirty) {
            river.update(victim.node, victim.pos);
        }
        cache_map.erase(cache_map.find(victim.pos));
        cache_list.pop_back();
    }
    cache_list.push_front({x, false, pos});
    cache_map[pos] = cache_list.begin();
    return x;
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::write_node(const Node& x,
                                                        int pos) {
    auto map_it = cache_map.find(pos);
    if (map_it != cache_map.end()) {
        auto old_list_it = map_it->second;
        bool current_dirty = old_list_it->dirty;
        cache_list.erase(old_list_it);
        cache_list.push_front({x, true, pos});
        cache_map[pos] = cache_list.begin();
    } else {
        if (cache_list.size() >= CACHE_CAPACITY) {
            CacheBlock& victim = cache_list.back();
            if (victim.dirty) {
                river.update(victim.node, victim.pos);
            }
            cache_map.erase(cache_map.find(victim.pos));
            cache_list.pop_back();
        }
        cache_list.push_front({x, true, pos});
        cache_map[pos] = cache_list.begin();
    }
}

template <typename IndexType, typename ValueType, int ORDER>
typename BPlusTree<IndexType, ValueType, ORDER>::Node
BPlusTree<IndexType, ValueType, ORDER>::add_to_cache(const Node& x, int pos) {
    if (cache_list.size() >= CACHE_CAPACITY) {
        CacheBlock& last = cache_list.back();
        if (last.dirty) {
            river.update(last.node, last.pos);
        }
        auto res = cache_map.find(last.pos);
        cache_map.erase(res);
        cache_list.pop_back();
    }
    cache_list.push_front({x, false, pos});
    cache_map[pos] = cache_list.begin();
    return x;
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::flush_all() {
    for (auto& block : cache_list) {
        if (block.dirty) {
            river.update(block.node, block.pos);
            block.dirty = false;
        }
    }
}

template <typename IndexType, typename ValueType, int ORDER>
int BPlusTree<IndexType, ValueType, ORDER>::min_leaf_keys() const {
    return (ORDER + 1) / 2;
}
template <typename IndexType, typename ValueType, int ORDER>
int BPlusTree<IndexType, ValueType, ORDER>::min_internal_keys() const {
    return (ORDER - 1) / 2;
}

template <typename IndexType, typename ValueType, int ORDER>
int BPlusTree<IndexType, ValueType, ORDER>::root() {
    int r;
    river.get_info(r, 1);
    return r;
}
template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::set_root(int r) {
    river.write_info(r, 1);
}

template <typename IndexType, typename ValueType, int ORDER>
int BPlusTree<IndexType, ValueType, ORDER>::new_node(const Node& x) {
    int pos = river.write(x);
    add_to_cache(x, pos);
    int cnt;
    river.get_info(cnt, 2);
    river.write_info(cnt + 1, 2);
    return pos;
}

template <typename IndexType, typename ValueType, int ORDER>
int BPlusTree<IndexType, ValueType, ORDER>::find_leaf(const Key& key) {
    int u = root();
    while (true) {
        Node x = node(u);
        if (x.is_leaf) return u;
        int i = 0;
        while (i < x.key_cnt && !(key < x.keys[i])) i++;
        u = x.child[i];
    }
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::insert_in_leaf(int u,
                                                            const Key& key) {
    Node x = node(u);
    int i = x.key_cnt;
    while (i > 0 && key < x.keys[i - 1]) {
        x.keys[i] = x.keys[i - 1];
        i--;
    }
    x.keys[i] = key;
    x.key_cnt++;
    write_node(x, u);
    if (x.key_cnt == ORDER) split_leaf(u);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::split_leaf(int u) {
    Node x = node(u), y;
    y.is_leaf = true;
    y.parent = x.parent;

    int mid = ORDER / 2;
    y.key_cnt = x.key_cnt - mid;
    for (int i = 0; i < y.key_cnt; i++) {
        y.keys[i] = x.keys[mid + i];
    }

    x.key_cnt = mid;
    y.next = x.next;
    x.next = new_node(y);
    write_node(x, u);
    write_node(y, x.next);

    insert_in_parent(u, y.keys[0], x.next);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::insert_in_parent(int u,
                                                              const Key& key,
                                                              int v) {
    if (u == root()) {
        Node r;
        r.is_leaf = false;
        r.key_cnt = 1;
        r.keys[0] = key;
        r.child[0] = u;
        r.child[1] = v;
        int rp = new_node(r);

        Node cu = node(u);
        cu.parent = rp;
        write_node(cu, u);
        Node cv = node(v);
        cv.parent = rp;
        write_node(cv, v);

        set_root(rp);
        return;
    }

    Node cur = node(u);
    int p = cur.parent;
    Node par = node(p);

    int i = par.key_cnt;
    while (i > 0 && key < par.keys[i - 1]) {
        par.keys[i] = par.keys[i - 1];
        par.child[i + 1] = par.child[i];
        i--;
    }
    par.keys[i] = key;
    par.child[i + 1] = v;
    par.key_cnt++;

    Node cv = node(v);
    cv.parent = p;
    write_node(cv, v);
    write_node(par, p);

    if (par.key_cnt == ORDER) split_internal(p);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::split_internal(int u) {
    Node x = node(u), y;
    y.is_leaf = false;
    y.parent = x.parent;

    int mid = ORDER / 2;
    Key up = x.keys[mid];

    y.key_cnt = x.key_cnt - mid - 1;
    for (int i = 0; i < y.key_cnt; i++) {
        y.keys[i] = x.keys[mid + 1 + i];
    }

    for (int i = 0; i <= y.key_cnt; i++) {
        y.child[i] = x.child[mid + 1 + i];
        Node c = node(y.child[i]);
        c.parent = new_node(y);
    }

    x.key_cnt = mid;
    int v = new_node(y);
    for (int i = 0; i <= y.key_cnt; i++) {
        Node c = node(y.child[i]);
        c.parent = v;
        write_node(c, y.child[i]);
    }

    write_node(x, u);
    write_node(y, v);
    insert_in_parent(u, up, v);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::fix_leaf(int u) {
    Node cur = node(u);
    int p = cur.parent;
    Node par = node(p);
    int idx = 0;
    while (par.child[idx] != u) idx++;

    if (idx > 0) {
        int l = par.child[idx - 1];
        Node left = node(l);
        if (left.key_cnt > min_leaf_keys()) {
            for (int i = cur.key_cnt; i > 0; i--) cur.keys[i] = cur.keys[i - 1];
            cur.keys[0] = left.keys[left.key_cnt - 1];
            cur.key_cnt++;
            left.key_cnt--;
            par.keys[idx - 1] = cur.keys[0];
            write_node(left, l);
            write_node(cur, u);
            write_node(par, p);
            return;
        }
    }

    if (idx + 1 <= par.key_cnt) {
        int r = par.child[idx + 1];
        Node right = node(r);
        if (right.key_cnt > min_leaf_keys()) {
            cur.keys[cur.key_cnt++] = right.keys[0];
            for (int i = 0; i + 1 < right.key_cnt; i++)
                right.keys[i] = right.keys[i + 1];
            right.key_cnt--;
            par.keys[idx] = right.keys[0];
            write_node(right, r);
            write_node(cur, u);
            write_node(par, p);
            return;
        }
    }

    if (idx > 0)
        merge_leaf(par.child[idx - 1], u, idx - 1);
    else
        merge_leaf(u, par.child[idx + 1], idx);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::merge_leaf(int l, int r, int sep) {
    Node left = node(l);
    Node right = node(r);
    Node par = node(left.parent);

    for (int i = 0; i < right.key_cnt; ++i)
        left.keys[left.key_cnt + i] = right.keys[i];
    left.key_cnt += right.key_cnt;
    left.next = right.next;
    for (int i = sep; i + 1 < par.key_cnt; ++i) {
        par.keys[i] = par.keys[i + 1];
        par.child[i + 1] = par.child[i + 2];
    }
    par.key_cnt--;

    write_node(left, l);
    write_node(par, left.parent);

    if (par.parent != -1 && par.key_cnt < min_leaf_keys())
        fix_internal(left.parent);

    if (par.key_cnt == 0 && left.parent == root()) {
        set_root(l);
        left.parent = -1;
        write_node(left, l);
    }
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::fix_internal(int u) {
    Node x = node(u);
    if (x.parent == -1 || x.key_cnt >= min_internal_keys()) return;

    Node par = node(x.parent);
    int idx = 0;
    while (par.child[idx] != u) idx++;

    if (idx > 0) {
        int l = par.child[idx - 1];
        Node left = node(l);
        if (left.key_cnt > min_internal_keys()) {
            for (int i = x.key_cnt; i > 0; i--) x.keys[i] = x.keys[i - 1];
            for (int i = x.key_cnt + 1; i > 0; i--) x.child[i] = x.child[i - 1];
            x.keys[0] = par.keys[idx - 1];
            x.child[0] = left.child[left.key_cnt];
            Node c = node(x.child[0]);
            c.parent = u;
            write_node(c, x.child[0]);
            par.keys[idx - 1] = left.keys[left.key_cnt - 1];
            x.key_cnt++;
            left.key_cnt--;
            write_node(left, l);
            write_node(x, u);
            write_node(par, x.parent);
            return;
        }
    }

    if (idx < par.key_cnt) {
        int r = par.child[idx + 1];
        Node right = node(r);
        if (right.key_cnt > min_internal_keys()) {
            x.keys[x.key_cnt] = par.keys[idx];
            x.child[x.key_cnt + 1] = right.child[0];
            Node c = node(x.child[x.key_cnt + 1]);
            c.parent = u;
            write_node(c, x.child[x.key_cnt + 1]);
            par.keys[idx] = right.keys[0];
            for (int i = 0; i + 1 < right.key_cnt; i++)
                right.keys[i] = right.keys[i + 1];
            for (int i = 0; i + 1 <= right.key_cnt; i++)
                right.child[i] = right.child[i + 1];
            x.key_cnt++;
            right.key_cnt--;
            write_node(right, r);
            write_node(x, u);
            write_node(par, x.parent);
            return;
        }
    }
    if (idx > 0)
        merge_internal(par.child[idx - 1], u, idx - 1);
    else
        merge_internal(u, par.child[idx + 1], idx);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::merge_internal(int l, int r,
                                                            int sep) {
    Node left = node(l);
    Node right = node(r);
    Node par = node(left.parent);

    left.keys[left.key_cnt++] = par.keys[sep];
    for (int i = 0; i < right.key_cnt; i++) {
        left.keys[left.key_cnt + i] = right.keys[i];
    }

    for (int i = 0; i <= right.key_cnt; i++) {
        left.child[left.key_cnt + i] = right.child[i];
        Node c = node(right.child[i]);
        c.parent = l;
        write_node(c, right.child[i]);
    }

    left.key_cnt += right.key_cnt;

    for (int i = sep; i + 1 < par.key_cnt; i++) {
        par.keys[i] = par.keys[i + 1];
        par.child[i + 1] = par.child[i + 2];
    }
    par.key_cnt--;

    write_node(left, l);
    write_node(par, left.parent);

    if (par.parent == -1 && par.key_cnt == 0) {
        set_root(l);
        left.parent = -1;
        write_node(left, l);
        return;
    }

    if (par.parent != -1 && par.key_cnt < min_internal_keys()) {
        fix_internal(left.parent);
    }
}
template <typename IndexType, typename ValueType, int ORDER>
BPlusTree<IndexType, ValueType, ORDER>::BPlusTree(string fn) {
    river.initialise(fn);
    int cnt;
    river.get_info(cnt, 2);
    if (cnt == 0) {
        Node r;
        r.is_leaf = true;
        int rp = river.write(r);
        river.write_info(rp, 1);
        river.write_info(1, 2);
    }
}

template <typename IndexType, typename ValueType, int ORDER>
BPlusTree<IndexType, ValueType, ORDER>::~BPlusTree() {
    flush_all();
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::insert(const IndexType& idx,
                                                    const ValueType& val) {
    Key key{};
    key.index = idx;
    key.value = val;
    int u = find_leaf(key);
    Node x = node(u);
    for (int i = 0; i < x.key_cnt; i++)
        if (x.keys[i] == key) return;
    insert_in_leaf(u, key);
}

template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::erase(const IndexType& idx,
                                                   const ValueType& val) {
    Key key{};
    key.index = idx;
    key.value = val;
    int u = find_leaf(key);
    Node x = node(u);
    int pos = -1;
    for (int i = 0; i < x.key_cnt; i++)
        if (x.keys[i] == key) pos = i;
    if (pos == -1) return;
    for (int i = pos; i + 1 < x.key_cnt; i++) x.keys[i] = x.keys[i + 1];
    x.key_cnt--;
    write_node(x, u);
    if (u != root() && x.key_cnt < min_leaf_keys()) fix_leaf(u);
}

template <typename IndexType, typename ValueType, int ORDER>
vector<typename BPlusTree<IndexType, ValueType, ORDER>::Key>
BPlusTree<IndexType, ValueType, ORDER>::find(const IndexType& idx) {
    vector<Key> res;
    Key low{};
    low.index = idx;
    low.value = numeric_limits<ValueType>::min();
    int u = find_leaf(low);
    while (u != -1) {
        Node x = node(u);
        for (int i = 0; i < x.key_cnt; i++) {
            if (x.keys[i].index == low.index) {
                res.push_back(x.keys[i]);
            } else if (x.keys[i].index > low.index) {
                break;
            }
        }
        u = x.next;
    }
    return res;
}
template <typename IndexType, typename ValueType, int ORDER>
void BPlusTree<IndexType, ValueType, ORDER>::clean_up() {
    cache_list.clear();
    cache_map.clear();

    river.clean_up();
    Node r;
    r.is_leaf = true;
    r.parent = -1;
    r.key_cnt = 0;
    
    int rp = river.write(r);     
    river.write_info(rp, 1);     
    river.write_info(1, 2);      
    add_to_cache(r, rp);
}// namespace sjtu
}