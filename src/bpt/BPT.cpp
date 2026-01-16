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

/* ================= MemoryRiver ================= */

struct MyString {
    static constexpr size_t MAX_LEN = 64;  // 最大字符长度
    char s[MAX_LEN + 1];                   // +1 留给 '\0'

    // 默认构造
    MyString() {
        s[0] = '\0';
    }

    // 从 std::string 构造
    MyString(const std::string& str) {
        size_t len = str.size() < MAX_LEN ? str.size() : MAX_LEN;
        std::memcpy(s, str.data(), len);
        s[len] = '\0';
    }

    // 赋值 std::string
    MyString& operator=(const std::string& str) {
        size_t len = str.size() < MAX_LEN ? str.size() : MAX_LEN;
        std::memcpy(s, str.data(), len);
        s[len] = '\0';
        return *this;
    }

    // 转 std::string
    std::string to_string() const {
        return std::string(s);
    }

    // 比较运算符
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

    // 辅助函数：最小值
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
    // 这个结构体用来表示 B+ Tree 中的每一个键值对
    // 包括索引 `index`（通常是用户提供的键），和对应的值 `value`
    // 为了支持高效比较，提供了 `operator==` 和 `operator<`
    struct Key {
        MyString index;   // 索引键（字符串）
        ValueType value;  // 相关的值

        // 判断两个 Key 是否相等：比较索引值和对应的值
        bool operator==(const Key& o) const {
            return std::string(index.s) == std::string(o.index.s) &&
                   value == o.value;
        }

        // 比较两个 Key 的大小：首先比较索引，如果相等，再比较值
        bool operator<(const Key& o) const {
            if (!(index == o.index))
                return std::string(index.s) < std::string(o.index.s);
            return value < o.value;
        }
    };

    // 代表 B+ Tree 中的节点
    // 每个节点可以是叶子节点或者内部节点，包含多个键值对和指向子节点的指针
    struct Node {
        bool is_leaf = false;  // 是否为叶子节点
        int key_cnt = 0;       // 当前节点中键值对的数量
        int parent = -1;       // 父节点的索引，根节点的 parent 为 -1
        int next = -1;         // 指向下一个叶子节点的指针，仅叶子节点有
        Key keys[ORDER];       // 当前节点存储的键值对
        int child[ORDER + 1];  // 指向子节点的指针，仅内部节点有
    };

    MemoryRiver<Node, 2> river;  // 用于读取和写入节点数据，缓存了两个信息槽

    /* ================= LRU Cache 实现 ================= */

    // 缓存块结构：每个缓存块代表磁盘中的一个节点，缓存为优化性能而设计。
    // dirty 标志位表示该缓存块是否已经被修改，pos 表示该节点在磁盘中的位置
    struct CacheBlock {
        Node node;           // 缓存的节点数据
        bool dirty = false;  // 标志此块是否被修改过
        int pos;             // 节点在磁盘中的位置
    };

    // 缓存容量：为了避免占用过多内存，设定最大缓存数量（可以根据内存限制调整）
    static constexpr int CACHE_CAPACITY =
        200;  // 最大缓存块数，减少磁盘 IO 的压力

    // 缓存使用 LRU（Least Recently Used）策略：
    // cache_list 保存缓存块，最近使用的块位于头部，最久未使用的块位于尾部。
    // cache_map 用来通过磁盘位置快速查找缓存块
    sjtu::list<CacheBlock> cache_list;
    sjtu::map<int, typename sjtu::list<CacheBlock>::iterator> cache_map;

    // 通过磁盘位置获取节点，首先检查缓存，如果缓存中存在该节点则返回缓存中的节点
    // 否则从磁盘读取节点，并将该节点添加到缓存中
    Node node(int pos) {
        // 1. 检查缓存
        auto it = cache_map.find(pos);
        if (it != cache_map.end()) {
            // 命中：将缓存块移动到链表头部（LRU）
            cache_list.insert(cache_list.begin(),
                              *it->second);  // 将当前元素插入到头部
            cache_list.erase(it->second);    // 从原位置删除
            return it->second->node;         // 返回缓存中的节点
        }

        // 2. 未命中：从磁盘读取
        Node x;
        river.read(x, pos);  // 通过 river 从磁盘读取节点

        // 3. 将新读取的节点添加到缓存
        return add_to_cache(x, pos);  // 添加到缓存并返回
    }

    // 将节点写入缓存（并标记为 dirty），如果缓存已满则驱逐最久未使用的节点
    void write_node(const Node& x, int pos) {
        auto it = cache_map.find(pos);
        if (it != cache_map.end()) {
            // 如果缓存中已存在该节点，更新节点并将其移动到链表头部
            it->second->node = x;
            it->second->dirty = true;                            // 标记为脏数据
            cache_list.insert(cache_list.begin(), *it->second);  // 插入到头部
            cache_list.erase(it->second);  // 删除原位置的节点
        } else {
            // 如果缓存中没有该节点，将其添加到缓存
            add_to_cache(x, pos);
            cache_list.begin()->dirty = true;  // 新加入的节点标记为脏数据
        }
    }

    // 将节点添加到缓存中，如果缓存已满则驱逐最久未使用的节点
    Node add_to_cache(const Node& x, int pos) {
        // 如果缓存已满，驱逐最久未使用的节点
        if (cache_list.size() >= CACHE_CAPACITY) {
            CacheBlock& last = cache_list.back();
            // 如果该节点是脏数据，则写回磁盘
            if (last.dirty) {
                river.update(last.node, last.pos);
            }
            auto res = cache_map.find(last.pos);
            cache_map.erase(res);   // 从哈希表中移除
            cache_list.pop_back();  // 从链表中移除
        }

        // 将新的节点插入到缓存的链表头部
        cache_list.push_front({x, false, pos});
        cache_map[pos] = cache_list.begin();  // 更新哈希表
        return x;                             // 返回新的节点
    }

    // 强制将所有脏数据写回磁盘。通常在 B+ Tree
    // 析构时调用，确保所有修改都保存到磁盘。
    void flush_all() {
        // 遍历缓存中的所有块，将脏数据写回磁盘
        for (auto& block : cache_list) {
            if (block.dirty) {
                river.update(block.node, block.pos);  // 写回磁盘
                block.dirty = false;                  // 清除脏标志
            }
        }
    }
    /* ================================================== */

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
        // 这里可以直接写入磁盘获取位置，也可以优化。
        // 为了简单，我们先写入磁盘获取 pos，然后立即放入缓存。
        int pos = river.write(x);

        // 立即加入缓存，避免后续马上读取时产生 IO
        // 注意：river.write 已经写了，所以 dirty=false
        add_to_cache(x, pos);

        int cnt;
        river.get_info(cnt, 2);
        river.write_info(cnt + 1, 2);
        return pos;
    }

    /* ========== 下面是原本的 B+ Tree 逻辑 (稍微保留原样) ========== */
    // 逻辑部分直接复制您的原始代码即可，因为现在 node() 和 write_node()
    // 已经被拦截处理了

    // 查找包含指定 key 的叶子节点
    // 从根节点开始，逐层向下查找，直到找到叶子节点
    int find_leaf(const Key& key) {
        int u = root();  // 从根节点开始查找
        while (true) {
            Node x = node(u);         // 获取当前节点
            if (x.is_leaf) return u;  // 如果是叶子节点，返回该节点
            int i = 0;
            while (i < x.key_cnt && !(key < x.keys[i]))
                i++;         // 查找匹配的子节点
            u = x.child[i];  // 向下查找
        }
    }

    // 向叶子节点中插入一个新的键值对
    void insert_in_leaf(int u, const Key& key) {
        Node x = node(u);
        int i = x.key_cnt;
        // 在叶子节点的键值对中插入，保持顺序
        while (i > 0 && key < x.keys[i - 1]) {
            x.keys[i] = x.keys[i - 1];
            i--;
        }
        x.keys[i] = key;   // 插入新的键值对
        x.key_cnt++;       // 增加键值对的数量
        write_node(x, u);  // 写入缓存

        // 如果节点已满，进行分裂
        if (x.key_cnt == ORDER) split_leaf(u);
    }

    // 处理叶子节点的分裂：当叶子节点满了后，需要将它分裂成两个节点
    void split_leaf(int u) {
        Node x = node(u), y;
        y.is_leaf = true;     // 新节点是叶子节点
        y.parent = x.parent;  // 新节点的父节点和原节点相同

        int mid = ORDER / 2;          // 分裂点在中间位置
        y.key_cnt = x.key_cnt - mid;  // 新节点包含后半部分的键值对
        for (int i = 0; i < y.key_cnt; i++) {
            y.keys[i] = x.keys[mid + i];  // 将后半部分的键值对复制到新节点
        }

        x.key_cnt = mid;  // 原节点只保留前半部分
        y.next = x.next;  // 新节点的 next 指针指向原节点的 next
        x.next =
            new_node(y);  // 在磁盘上为新节点分配位置，并将其添加到原节点的 next
        write_node(x, u);       // 写回原节点
        write_node(y, x.next);  // 写回新节点

        insert_in_parent(u, y.keys[0],
                         x.next);  // 在父节点中插入新的键值对（分裂产生的键）
    }

    // 在父节点中插入新的键值对，如果是根节点分裂，创建新的根
    void insert_in_parent(int u, const Key& key, int v) {
        if (u == root()) {
            // 如果是根节点，需要创建新的根节点
            Node r;
            r.is_leaf = false;  // 新根节点是内部节点
            r.key_cnt = 1;
            r.keys[0] = key;       // 新根节点存储分裂产生的键
            r.child[0] = u;        // 原来的根节点成为新根的左子树
            r.child[1] = v;        // 新节点成为新根的右子树
            int rp = new_node(r);  // 创建新根节点

            // 更新原来两个子节点的父节点为新根
            Node cu = node(u);
            cu.parent = rp;
            write_node(cu, u);
            Node cv = node(v);
            cv.parent = rp;
            write_node(cv, v);

            set_root(rp);  // 更新树的根节点
            return;
        }

        Node cur = node(u);
        int p = cur.parent;  // 获取父节点
        Node par = node(p);

        int i = par.key_cnt;
        while (i > 0 && key < par.keys[i - 1]) {
            par.keys[i] = par.keys[i - 1];
            par.child[i + 1] = par.child[i];
            i--;
        }
        par.keys[i] = key;     // 将新键插入到父节点
        par.child[i + 1] = v;  // 新节点成为父节点的右子树
        par.key_cnt++;         // 更新父节点的键数量

        Node cv = node(v);
        cv.parent = p;
        write_node(cv, v);
        write_node(par, p);  // 更新父节点

        if (par.key_cnt == ORDER)
            split_internal(p);  // 如果父节点满了，进行分裂
    }

    // 分裂内部节点：当内部节点的键数超过限制时，需要进行分裂
    void split_internal(int u) {
        Node x = node(u), y;
        y.is_leaf = false;    // 新节点是内部节点
        y.parent = x.parent;  // 新节点的父节点和原节点相同

        int mid = ORDER / 2;   // 分裂点
        Key up = x.keys[mid];  // 保留中间的键，提升到父节点

        y.key_cnt = x.key_cnt - mid - 1;  // 新节点包含后半部分的键
        for (int i = 0; i < y.key_cnt; i++) {
            y.keys[i] = x.keys[mid + 1 + i];  // 将右半部分的键值对复制到新节点
        }

        for (int i = 0; i <= y.key_cnt; i++) {
            y.child[i] = x.child[mid + 1 + i];  // 复制子节点
            Node c = node(y.child[i]);
            c.parent = new_node(y);  // 新分配的节点应更新子节点的父节点
        }

        x.key_cnt = mid;      // 更新原节点的键数量
        int v = new_node(y);  // 为新节点分配磁盘位置
        // 更新子节点的父节点信息
        for (int i = 0; i <= y.key_cnt; i++) {
            Node c = node(y.child[i]);
            c.parent = v;
            write_node(c, y.child[i]);
        }

        write_node(x, u);            // 更新原节点
        write_node(y, v);            // 写回新节点
        insert_in_parent(u, up, v);  // 将中间的键提升到父节点
    }

    // 修复叶子节点：处理删除后叶子节点的下溢问题
    void fix_leaf(int u) {
        Node cur = node(u);
        int p = cur.parent;
        Node par = node(p);
        int idx = 0;
        while (par.child[idx] != u) idx++;  // 查找当前节点在父节点中的位置

        // 如果左边的兄弟节点有足够的键，则进行借位
        if (idx > 0) {
            int l = par.child[idx - 1];
            Node left = node(l);
            if (left.key_cnt > min_leaf_keys()) {
                // 借位操作：将左边的最后一个键借到当前节点
                for (int i = cur.key_cnt; i > 0; i--)
                    cur.keys[i] = cur.keys[i - 1];
                cur.keys[0] = left.keys[left.key_cnt - 1];
                cur.key_cnt++;
                left.key_cnt--;
                par.keys[idx - 1] = cur.keys[0];  // 更新父节点的键
                write_node(left, l);
                write_node(cur, u);
                write_node(par, p);
                return;
            }
        }

        // 如果右边的兄弟节点有足够的键，则进行借位
        if (idx + 1 <= par.key_cnt) {
            int r = par.child[idx + 1];
            Node right = node(r);
            if (right.key_cnt > min_leaf_keys()) {
                // 借位操作：将右边的第一个键借到当前节点
                cur.keys[cur.key_cnt++] = right.keys[0];
                for (int i = 0; i + 1 < right.key_cnt; i++)
                    right.keys[i] = right.keys[i + 1];
                right.key_cnt--;
                par.keys[idx] = right.keys[0];  // 更新父节点的键
                write_node(right, r);
                write_node(cur, u);
                write_node(par, p);
                return;
            }
        }

        // 如果左右兄弟节点都没有足够的键，则进行合并
        if (idx > 0)
            merge_leaf(par.child[idx - 1], u, idx - 1);
        else
            merge_leaf(u, par.child[idx + 1], idx);
    }

    // 合并叶子节点：当一个叶子节点的键数过少时，从兄弟节点合并键值对
    void merge_leaf(int l, int r, int sep) {
        Node left = node(l);
        Node right = node(r);
        Node par = node(left.parent);

        // 将右兄弟的所有键值对移到左节点中
        for (int i = 0; i < right.key_cnt; ++i)
            left.keys[left.key_cnt + i] = right.keys[i];
        left.key_cnt += right.key_cnt;  // 更新左节点的键数
        left.next = right.next;         // 更新 next 指针
        // 从父节点中移除合并的键
        for (int i = sep; i + 1 < par.key_cnt; ++i) {
            par.keys[i] = par.keys[i + 1];
            par.child[i + 1] = par.child[i + 2];
        }
        par.key_cnt--;  // 更新父节点的键数

        write_node(left, l);           // 更新左节点
        write_node(par, left.parent);  // 更新父节点

        // 如果父节点需要修复，递归修复
        if (par.parent != -1 && par.key_cnt < min_leaf_keys())
            fix_internal(left.parent);

        // 如果父节点已经空了且它是根节点，更新树根
        if (par.key_cnt == 0 && left.parent == root()) {
            set_root(l);  // 设置新的根节点
            left.parent = -1;
            write_node(left, l);  // 更新左节点
        }
    }

    // 修复内部节点：处理删除后内部节点的下溢问题
    void fix_internal(int u) {
        Node x = node(u);
        if (x.parent == -1 || x.key_cnt >= min_internal_keys())
            return;  // 如果是根节点或者节点键数正常，则不需要修复

        Node par = node(x.parent);
        int idx = 0;
        while (par.child[idx] != u) idx++;  // 查找当前节点在父节点中的位置

        // 如果左边的兄弟节点有足够的键，则进行借位
        if (idx > 0) {
            int l = par.child[idx - 1];
            Node left = node(l);
            if (left.key_cnt > min_internal_keys()) {
                // 借位操作：将左边的最后一个键借到当前节点
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

        // 如果右边的兄弟节点有
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

    // 合并内部节点：当内部节点的键数过少时，从兄弟节点合并键值对
    void merge_internal(int l, int r, int sep) {
        Node left = node(l);           // 获取左兄弟节点
        Node right = node(r);          // 获取右兄弟节点
        Node par = node(left.parent);  // 获取父节点

        left.keys[left.key_cnt++] =
            par.keys[sep];  // 将父节点的分隔键插入到左节点
        // 将右兄弟节点的所有键值对复制到左节点
        for (int i = 0; i < right.key_cnt; i++) {
            left.keys[left.key_cnt + i] = right.keys[i];
        }

        // 将右兄弟节点的所有子节点指针复制到左节点
        for (int i = 0; i <= right.key_cnt; i++) {
            left.child[left.key_cnt + i] = right.child[i];
            Node c = node(right.child[i]);
            c.parent = l;                   // 更新子节点的父节点
            write_node(c, right.child[i]);  // 写回子节点
        }

        left.key_cnt += right.key_cnt;  // 更新左节点的键数量

        // 从父节点中移除分隔键，并更新父节点的子节点指针
        for (int i = sep; i + 1 < par.key_cnt; i++) {
            par.keys[i] = par.keys[i + 1];
            par.child[i + 1] = par.child[i + 2];
        }
        par.key_cnt--;  // 更新父节点的键数量

        write_node(left, l);           // 写回左节点
        write_node(par, left.parent);  // 写回父节点

        // 如果父节点是根节点并且键数量为零，则更新树的根节点
        if (par.parent == -1 && par.key_cnt == 0) {
            set_root(l);          // 设置新的根节点
            left.parent = -1;     // 设置左节点为新的根节点
            write_node(left, l);  // 写回左节点
            return;
        }

        // 如果父节点不是根节点，并且父节点键数量小于最小阈值，则修复父节点
        if (par.parent != -1 && par.key_cnt < min_internal_keys()) {
            fix_internal(left.parent);  // 修复父节点
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

    // 析构函数：非常重要！必须把缓存里的脏数据写回磁盘
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
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    // ... main 函数保持不变 ...
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
