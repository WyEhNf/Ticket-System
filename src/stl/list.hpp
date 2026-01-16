#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP
#include <cstddef>
#include <exception>
#include <iostream>

namespace sjtu {
template <typename T>
class list {
protected:
    class node {
    public:
        T val_;
        node* nxt_;
        node* pre_;

        node() = delete;
        node(const T& val, node* nxt, node* pre) : val_(val), nxt_(nxt), pre_(pre) {}
        node(T&& val, node* nxt, node* pre) : val_(std::move(val)), nxt_(nxt), pre_(pre) {}

        // 提供访问函数来获取 ptr_
        node* get_ptr() { return this; }
    };

protected:
    size_t size_;
    node* head_;
    node* tail_;

    // 插入一个新节点到指定位置前
    node* insert(node* pos, node* cur) {
        pos->pre_->nxt_ = cur;
        cur->pre_ = pos->pre_;
        pos->pre_ = cur;
        cur->nxt_ = pos;
        return cur;
    }

    // 删除指定节点
    node* erase(node* pos) {
        pos->pre_->nxt_ = pos->nxt_;
        pos->nxt_->pre_ = pos->pre_;
        return pos;
    }

public:
    class const_iterator;
    class iterator {
    private:
        node* ptr_;
    public:
        explicit iterator(node* ptr = nullptr) : ptr_(ptr) {}

        node* get_ptr() const { return ptr_; }  // 提供一个访问函数

        iterator operator++(int) { auto tmp = *this; ptr_ = ptr_->nxt_; return tmp; }
        iterator& operator++() { ptr_ = ptr_->nxt_; return *this; }
        iterator operator--(int) { auto tmp = *this; ptr_ = ptr_->pre_; return tmp; }
        iterator& operator--() { ptr_ = ptr_->pre_; return *this; }
        T& operator*() const { return ptr_->val_; }
        T* operator->() const noexcept { return &ptr_->val_; }

        bool operator==(const iterator& rhs) const { return ptr_ == rhs.ptr_; }
        bool operator!=(const iterator& rhs) const { return ptr_ != rhs.ptr_; }
    };

    class const_iterator {
    private:
        const node* ptr_;
    public:
        explicit const_iterator(node* ptr = nullptr) : ptr_(ptr) {}

        const_iterator operator++(int) { auto tmp = *this; ptr_ = ptr_->nxt_; return tmp; }
        const_iterator& operator++() { ptr_ = ptr_->nxt_; return *this; }
        const_iterator operator--(int) { auto tmp = *this; ptr_ = ptr_->pre_; return tmp; }
        const_iterator& operator--() { ptr_ = ptr_->pre_; return *this; }
        const T& operator*() const { return ptr_->val_; }
        T* operator->() const noexcept { return &ptr_->val_; }

        bool operator==(const iterator& rhs) const { return ptr_ == rhs.ptr_; }
        bool operator!=(const iterator& rhs) const { return ptr_ != rhs.ptr_; }
    };

    // 默认构造函数
    list() : size_(0) {
        head_ = static_cast<node*>(operator new(sizeof(node)));
        tail_ = static_cast<node*>(operator new(sizeof(node)));
        head_->nxt_ = tail_;
        head_->pre_ = nullptr;
        tail_->pre_ = head_;
        tail_->nxt_ = nullptr;
    }

    // 插入元素到特定位置前
    iterator insert(iterator pos, const T& value) {
        if (pos.get_ptr() == head_ || pos.get_ptr() == nullptr) {
            throw std::exception();
        }
        ++size_;
        return iterator(insert(pos.get_ptr(), new node(value, nullptr, nullptr)));
    }

    // 删除指定位置的元素
    iterator erase(iterator pos) {
        if (pos.get_ptr() == head_ || pos.get_ptr() == tail_ || pos.get_ptr() == nullptr) {
            throw std::exception();
        }
        --size_;
        node* tmp = pos.get_ptr()->nxt_;
        delete erase(pos.get_ptr());
        return iterator(tmp);
    }

    // 在末尾插入元素
    void push_back(const T& value) {
        ++size_;
        insert(tail_, new node(value, nullptr, nullptr));
    }

    // 删除最后一个元素
    void pop_back() {
        if (size_ == 0) {
            throw std::exception();
        }
        --size_;
        delete erase(tail_->pre_);
    }

    // 检查链表是否为空
    bool empty() const { return head_->nxt_ == tail_; }

    // 返回链表大小
    size_t size() const { return size_; }

    // 清空链表
    void clear() {
        while (head_->nxt_ != tail_) {
            delete erase(head_->nxt_);
        }
        size_ = 0;
    }

    // 获取链表头部元素
    const T& front() const {
        if (head_->nxt_ == tail_) {
            throw std::exception();
        }
        return head_->nxt_->val_;
    }

    // 获取链表尾部元素
    const T& back() const {
        if (head_->nxt_ == tail_) {
            throw std::exception();
        }
        return tail_->pre_->val_;
    }

    // 获取链表的迭代器
    iterator begin() { return iterator(head_->nxt_); }
    iterator end() { return iterator(tail_); }
    const_iterator cbegin() const { return const_iterator(head_->nxt_); }
    const_iterator cend() const { return const_iterator(tail_); }
};

}  //  namespace sjtu

#endif  // SJTU_LIST_HPP
