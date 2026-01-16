#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include <iostream>
using namespace std;

namespace sjtu {

template <typename T>
class list {
private:
    // 内部节点定义
    template <typename U>
    struct ListNode {
        T data;          // 存储数据
        ListNode* next;  // 指向下一个节点的指针
        ListNode* prev;  // 指向前一个节点的指针

        // 构造函数
        ListNode(T val) : data(val), next(nullptr), prev(nullptr) {}
    };

    ListNode<T>* head;  // 链表的头指针
    ListNode<T>* tail;  // 链表的尾指针
    int size_;           // 链表的元素个数

public:
    // 构造函数
    list() : head(nullptr), tail(nullptr), size_(0) {}

    // 析构函数
    ~list() {
        clear();
    }

    // 在链表尾部插入元素
    void push_back(T val) {
        ListNode<T>* newListNode = new ListNode<T>(val);
        if (tail) {
            tail->next = newListNode;
            newListNode->prev = tail;
        } else {
            head = newListNode;  // 链表为空时，新节点就是头节点
        }
        tail = newListNode;
        size_++;
    }

    // 在链表头部插入元素
    void push_front(T val) {
        ListNode<T>* newListNode = new ListNode<T>(val);
        if (head) {
            newListNode->next = head;
            head->prev = newListNode;
        } else {
            tail = newListNode;  // 链表为空时，新节点就是尾节点
        }
        head = newListNode;
        size_++;
    }

    // 删除链表的头节点
    void pop_front() {
        if (head) {
            ListNode<T>* temp = head;
            head = head->next;
            if (head) head->prev = nullptr; // 如果链表不为空，更新头节点的prev指针
            else tail = nullptr;  // 如果链表为空，尾指针也应为 nullptr
            delete temp;
            size_--;
        }
    }

    // 删除链表的尾节点
    void pop_back() {
        if (tail) {
            if (head == tail) {  // 只有一个节点的情况
                delete head;
                head = tail = nullptr;
            } else {
                ListNode<T>* temp = tail;
                tail = tail->prev;
                tail->next = nullptr;
                delete temp;
            }
            size_--;
        }
    }

    // 查找链表中的元素
    ListNode<T>* find(T val) {
        ListNode<T>* temp = head;
        while (temp) {
            if (temp->data == val) return temp;
            temp = temp->next;
        }
        return nullptr;  // 没有找到
    }

    // 清空链表
    void clear() {
        while (head) {
            pop_front();
        }
    }

    // 输出链表元素
    void print() {
        ListNode<T>* temp = head;
        while (temp) {
            cout << temp->data << " ";
            temp = temp->next;
        }
        cout << endl;
    }

    // 获取链表的大小
    int size() const {
        return size_;
    }

    // 判断链表是否为空
    bool empty() const {
        return size_ == 0;
    }

    // 返回指向链表第一个元素的迭代器
    class iterator {
    private:
        ListNode<T>* ptr_;  // 当前指针

    public:
        // 构造函数
        iterator(ListNode<T>* ptr = nullptr) : ptr_(ptr) {}

        // 前进
        iterator& operator++() {
            if (ptr_) {
                ptr_ = ptr_->next;
            }
            return *this;
        }

        // 后退
        iterator& operator--() {
            if (ptr_) {
                ptr_ = ptr_->prev;
            }
            return *this;
        }

        // 前进（返回迭代器，适用于后缀递增）
        iterator operator++(int) {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // 后退（返回迭代器，适用于后缀递减）
        iterator operator--(int) {
            iterator tmp(*this);
            --(*this);
            return tmp;
        }

        // 解引用操作符
        T& operator*() const {
            return ptr_->data;
        }

        // `->` 操作符，返回当前指针的地址
        T* operator->() const {
            return &(ptr_->data);
        }

        // 比较迭代器是否相等
        bool operator==(const iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const iterator& other) const {
            return ptr_ != other.ptr_;
        }

        // 获取当前节点
        ListNode<T>* getNode() const { return ptr_; }
    };

    // 返回指向头部的迭代器
    iterator begin() {
        return iterator(head);
    }
    T& back() {
        return tail->data;
    }
    // 返回指向尾部的迭代器（尾后迭代器）
    iterator end() {
        return iterator(nullptr);  // 尾后迭代器
    }

    // splice: 将源链表中的某个元素（或范围）插入到当前链表的指定位置
    void splice(iterator position, list<T>& other, iterator node_to_move) {
        ListNode<T>* ListNode_to_move = node_to_move.getNode();
        if (!ListNode_to_move || !other.head) return;

        // 找到 ListNode_to_move 的前一个节点
        ListNode<T>* prev = nullptr;
        ListNode<T>* temp = other.head;
        while (temp && temp != ListNode_to_move) {
            prev = temp;
            temp = temp->next;
        }

        if (temp == nullptr) return; // 找不到节点

        // 将 ListNode_to_move 从源链表中移除
        if (prev) {
            prev->next = ListNode_to_move->next;
        } else {
            other.head = ListNode_to_move->next; // 如果 ListNode_to_move 是头节点
        }
        if (other.tail == ListNode_to_move) {
            other.tail = prev; // 如果 ListNode_to_move 是尾节点
        }

        // 将 ListNode_to_move 插入到目标链表的 position 前面
        ListNode_to_move->next = position.getNode();
        if (position == begin()) {
            head = ListNode_to_move;
        } else {
            ListNode<T>* p = head;
            while (p && p->next != position.getNode()) {
                p = p->next;
            }
            if (p) p->next = ListNode_to_move;
        }

        // 更新尾指针
        if (!ListNode_to_move->next) {
            tail = ListNode_to_move;
        }

        size_++;
        other.size_--;
    }
};

}

#endif
