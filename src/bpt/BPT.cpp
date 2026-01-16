#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "list.hpp"
#include "map.hpp"


using namespace std;
using namespace sjtu;

struct MyString {
    static constexpr size_t MAX_LEN = 64;
    char s[MAX_LEN + 1];

    MyString() {
        s[0] = '\0';
    }

    MyString(const std::string& str) {
        size_t len = str.size() < MAX_LEN ? str.size() : MAX_LEN;
        std::memcpy(s, str.data(), len);
        s[len] = '\0';
    }

    MyString& operator=(const std::string& str) {
        size_t len = str.size() < MAX_LEN ? str.size() : MAX_LEN;
        std::memcpy(s, str.data(), len);
        s[len] = '\0';
        return *this;
    }

    std::string to_string() const {
        return std::string(s);
    }

    bool operator==(const MyString& o) const {
        return std::strcmp(s, o.s) == 0;
    }
    bool operator!=(const MyString& o) const {
        return !(*this == o);
    }
    bool operator<(const MyString& o) const {
        return std::strcmp(s, o.s) < 0;
    }
    bool operator>(const MyString& o) const {
        return std::strcmp(s, o.s) > 0;
    }
    bool operator<=(const MyString& o) const {
        return !(*this > o);
    }
    bool operator>=(const MyString& o) const {
        return !(*this < o);
    }

    static MyString min_value() {
        MyString ms;
        ms.s[0] = '\0';
        return ms;
    }
};

template <class T, int info_len = 2>
class MemoryRiver {
   private:
    fstream file;
    string file_name;

   public:
    MemoryRiver() = default;
    MemoryRiver(const string& fn) : file_name(fn) {
    }
    ~MemoryRiver() {
        if (file.is_open()) file.close();
    }

    void initialise(string fn = "") {
        if (!fn.empty()) file_name = fn;
        if (!filesystem::exists(file_name)) {
            file.open(file_name, ios::binary | ios::in | ios::out | ios::trunc);
            int zero = 0;
            for (int i = 0; i < info_len; i++)
                file.write(reinterpret_cast<char*>(&zero), sizeof(int));
            file.close();
        }
        file.open(file_name, ios::binary | ios::in | ios::out);
    }

    void get_info(int& x, int n) {
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char*>(&x), sizeof(int));
    }

    void write_info(int x, int n) {
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char*>(&x), sizeof(int));
    }

    int write(const T& t) {
        file.seekp(0, ios::end);
        int pos = file.tellp();
        file.write(reinterpret_cast<const char*>(&t), sizeof(T));
        return pos;
    }

    void read(T& t, int pos) {
        file.seekg(pos);
        file.read(reinterpret_cast<char*>(&t), sizeof(T));
    }

    void update(const T& t, int pos) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char*>(&t), sizeof(T));
    }
};

template <typename IndexType, typename ValueType, int ORDER = 32>
class BPlusTree {
   private:
    struct Key {
        MyString index;
        ValueType value;

        bool operator==(const Key& o) const {
            return std::string(index.s) == std::string(o.index.s) &&
                   value == o.value;
        }

        bool operator<(const Key& o) const {
            if (!(index == o.index))
                return std::string(index.s) < std::string(o.index.s);
            return value < o.value;
        }
    };

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

    sjtu::list<CacheBlock> cache_list;
    sjtu::map<int, typename sjtu::list<CacheBlock>::iterator> cache_map;

    Node node(int pos) {
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

    void write_node(const Node& x, int pos) {
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

    Node add_to_cache(const Node& x, int pos) {
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

    void flush_all() {
        for (auto& block : cache_list) {
            if (block.dirty) {
                river.update(block.node, block.pos);
                block.dirty = false;
            }
        }
    }

    int min_leaf_keys() const {
        return (ORDER + 1) / 2;
    }
    int min_internal_keys() const {
        return (ORDER - 1) / 2;
    }

    int root() {
        int r;
        river.get_info(r, 1);
        return r;
    }
    void set_root(int r) {
        river.write_info(r, 1);
    }

    int new_node(const Node& x) {
        int pos = river.write(x);
        add_to_cache(x, pos);
        int cnt;
        river.get_info(cnt, 2);
        river.write_info(cnt + 1, 2);
        return pos;
    }

    int find_leaf(const Key& key) {
        int u = root();
        while (true) {
            Node x = node(u);
            if (x.is_leaf) return u;
            int i = 0;
            while (i < x.key_cnt && !(key < x.keys[i])) i++;
            u = x.child[i];
        }
    }

    void insert_in_leaf(int u, const Key& key) {
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

    void split_leaf(int u) {
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

    void insert_in_parent(int u, const Key& key, int v) {
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

    void split_internal(int u) {
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

    void fix_leaf(int u) {
        Node cur = node(u);
        int p = cur.parent;
        Node par = node(p);
        int idx = 0;
        while (par.child[idx] != u) idx++;

        if (idx > 0) {
            int l = par.child[idx - 1];
            Node left = node(l);
            if (left.key_cnt > min_leaf_keys()) {
                for (int i = cur.key_cnt; i > 0; i--)
                    cur.keys[i] = cur.keys[i - 1];
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

    void merge_leaf(int l, int r, int sep) {
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

    void fix_internal(int u) {
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
                for (int i = x.key_cnt + 1; i > 0; i--)
                    x.child[i] = x.child[i - 1];
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

    void merge_internal(int l, int r, int sep) {
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

   public:
    BPlusTree(string fn = "bpt.db") {
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

    ~BPlusTree() {
        flush_all();
    }

    void insert(const IndexType& idx, const ValueType& val) {
        Key key{};
        key.index = idx;
        key.value = val;
        int u = find_leaf(key);
        Node x = node(u);
        for (int i = 0; i < x.key_cnt; i++)
            if (x.keys[i] == key) return;
        insert_in_leaf(u, key);
    }

    void erase(const IndexType& idx, const ValueType& val) {
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

    void find(const IndexType& idx) {
        Key low{};
        low.index = idx;
        low.value = numeric_limits<ValueType>::min();
        int u = find_leaf(low);
        bool found = false;
        while (u != -1) {
            Node x = node(u);
            for (int i = 0; i < x.key_cnt; i++) {
                if (x.keys[i].index == low.index) {
                    if (found) cout << ' ';
                    cout << x.keys[i].value;
                    found = true;
                } else if (x.keys[i].index > low.index) {
                    cout << (found ? "" : "null") << '\n';
                    return;
                }
            }
            u = x.next;
        }
        cout << (found ? "" : "null") << '\n';
    }
};

int main() {
    BPlusTree<MyString, int> db;
    int n;
    cin >> n;
    while (n--) {
        string op;
        cin >> op;
        if (op == "insert") {
            string idx;
            int val;
            cin >> idx >> val;
            db.insert(idx, val);
        } else if (op == "delete") {
            string idx;
            int val;
            cin >> idx >> val;
            db.erase(idx, val);
        } else {
            string idx;
            cin >> idx;
            db.find(idx);
        }
    }
    return 0;
}