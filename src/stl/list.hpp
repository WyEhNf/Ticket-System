#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP
#include <cstddef>
#include <exception>
#include <iostream>


namespace sjtu {

/**
 * a data container like std::list
 * allocate random memory addresses for data and they are doubly-linked in a
 * list.
 */
template <typename T>
class list {
   protected:
    class node {
       public:
        /**
         * add data members and constructors & destructor
         */
        T val_;
        node *nxt_, *pre_;
        node() = delete;
        node(const T& val, node* nxt, node* pre)
            : val_(val), nxt_(nxt), pre_(pre) {
        }
        node(T&& val, node* nxt, node* pre)
            : val_(std::move(val)), nxt_(nxt), pre_(pre) {
        }
    };

   protected:
    /**
     * add data members for linked list as protected members
     */
    size_t size_;
    node *head_, *tail_;
    /**
     * insert node cur before node pos
     * return the inserted node cur
     */
    node* insert(node* pos, node* cur) {
        pos->pre_->nxt_ = cur;
        cur->pre_ = pos->pre_;
        pos->pre_ = cur;
        cur->nxt_ = pos;
        return cur;
    }

    /**
     * remove node pos from list (no need to delete the node)
     * return the removed node pos
     */
    node* erase(node* pos) {
        pos->pre_->nxt_ = pos->nxt_;
        pos->nxt_->pre_ = pos->pre_;
        return pos;
    }

   public:
    class const_iterator;

    class iterator {
        friend class list;
        friend class const_iterator;

       private:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
        node* ptr_;

       public:
        explicit iterator(node* ptr = nullptr) : ptr_(ptr) {
        }
        iterator operator++(int) {
            auto tmp = *this;
            ptr_ = ptr_->nxt_;
            return tmp;
        }

        iterator& operator++() {
            ptr_ = ptr_->nxt_;
            return *this;
        }

        iterator operator--(int) {
            auto tmp = *this;
            ptr_ = ptr_->pre_;
            return tmp;
        }

        iterator& operator--() {
            ptr_ = ptr_->pre_;
            return *this;
        }

        /**
         * TODO *it
         * throw std::exception if iterator is invalid
         */
        T& operator*() const {
            if (ptr_ == nullptr || ptr_->nxt_ == nullptr ||
                ptr_->pre_ == nullptr) {
                throw std::exception();
            }
            return ptr_->val_;
        }

        /**
         * TODO it->field
         * throw std::exception if iterator is invalid
         */
        T* operator->() const noexcept {
            if (ptr_ == nullptr || ptr_->nxt_ == nullptr ||
                ptr_->pre_ == nullptr) {
                throw std::exception();
            }
            return &ptr_->val_;
        }

        /**
         * a operator to check whether two iterators are same (pointing to the
         * same memory).
         */
        bool operator==(const iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }

        bool operator==(const const_iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }

        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator& rhs) const {
            return !operator==(rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !operator==(rhs);
        }
    };

    /**
     * TODO
     * has same function as iterator, just for a const object.
     * should be able to construct from an iterator.
     */
    class const_iterator {
        friend class iterator;

       private:
        const node* ptr_;

       public:
        explicit const_iterator(node* ptr = nullptr) : ptr_(ptr) {
        }

        const_iterator operator++(int) {
            auto tmp = *this;
            ptr_ = ptr_->nxt_;
            return tmp;
        }

        const_iterator& operator++() {
            ptr_ = ptr_->nxt_;
            return *this;
        }

        const_iterator operator--(int) {
            auto tmp = *this;
            ptr_ = ptr_->pre_;
            return tmp;
        }

        const_iterator& operator--() {
            ptr_ = ptr_->pre_;
            return *this;
        }

        const T& operator*() const {
            if (ptr_ == nullptr || ptr_->nxt_ == nullptr ||
                ptr_->pre_ == nullptr) {
                throw std::exception();
            }
            return ptr_->val_;
        }

        T* operator->() const noexcept {
            if (ptr_ == nullptr || ptr_->nxt_ == nullptr ||
                ptr_->pre_ == nullptr) {
                throw std::exception();
            }
            return &ptr_->val_;
        }

        bool operator==(const iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }

        bool operator==(const const_iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }

        bool operator!=(const iterator& rhs) const {
            return !operator==(rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !operator==(rhs);
        }
    };

    /**
     * TODO Constructs
     * Atleast two: default constructor, copy constructor
     */
    list() {
        size_ = 0;
        head_ = static_cast<node*>(operator new(sizeof(node)));
        tail_ = static_cast<node*>(operator new(sizeof(node)));
        head_->nxt_ = tail_;
        head_->pre_ = nullptr;
        tail_->pre_ = head_;
        tail_->nxt_ = nullptr;
    }

    list(const list& other) {
        size_ = other.size_;
        head_ = static_cast<node*>(operator new(sizeof(node)));
        tail_ = static_cast<node*>(operator new(sizeof(node)));
        head_->nxt_ = tail_;
        head_->pre_ = nullptr;
        tail_->pre_ = head_;
        tail_->nxt_ = nullptr;

        node* cur = other.head_->nxt_;
        while (cur != other.tail_) {
            insert(tail_, new node(*cur));
            cur = cur->nxt_;
        }
    }

    /**
     * TODO Destructor
     */
    virtual ~list() {
        while (head_->nxt_ != tail_) {
            delete erase(head_->nxt_);
        }
        operator delete(head_);
        operator delete(tail_);
    }

    /**
     * TODO Assignment operator
     */
    list& operator=(const list& other) {
        if (&other == this) {
            return *this;
        }
        size_ = other.size_;
        while (head_->nxt_ != tail_) {
            delete erase(head_->nxt_);
        }
        node* cur = other.head_->nxt_;
        while (cur != other.tail_) {
            insert(tail_, new node(*cur));
            cur = cur->nxt_;
        }
        return *this;
    }

    /**
     * access the first / last element
     * throw container_is_empty when the container is empty.
     */
    const T& front() const {
        if (head_->nxt_ == tail_) {
            throw std::exception();
        }
        return head_->nxt_->val_;
    }

    const T& back() const {
        if (head_->nxt_ == tail_) {
            throw std::exception();
        }
        return tail_->pre_->val_;
    }

    /**
     * returns an iterator to the beginning.
     */
    iterator begin() {
        return iterator(head_->nxt_);
    }

    const_iterator cbegin() const {
        return const_iterator(head_->nxt_);
    }

    /**
     * returns an iterator to the end.
     */
    iterator end() {
        return iterator(tail_);
    }

    const_iterator cend() const {
        return const_iterator(tail_);
    }

    /**
     * checks whether the container is empty.
     */
    virtual bool empty() const {
        return head_->nxt_ == tail_;
    }

    /**
     * returns the number of elements
     */
    virtual size_t size() const {
        return size_;
    }

    /**
     * clears the contents
     */
    virtual void clear() {
        while (head_->nxt_ != tail_) {
            delete erase(head_->nxt_);
        }
        size_ = 0;
    }

    /**
     * insert value before pos (pos may be the end() iterator)
     * return an iterator pointing to the inserted value
     * throw if the iterator is invalid
     */
    // virtual iterator insert(iterator pos, const T &value) {
    //   if (pos.ptr_ == head_ || pos.ptr_ == nullptr) {
    //     throw std::exception();
    //   }
    //   ++size_;
    //   return iterator(insert(pos.ptr_, new node(value, nullptr, nullptr)));
    // }

    /**
     * remove the element at pos (the end() iterator is invalid)
     * returns an iterator pointing to the following element, if pos pointing to
     * the last element, end() will be returned. throw if the container is
     * empty, the iterator is invalid
     */
    virtual iterator erase(iterator pos) {
        if (pos.ptr_ == head_ || pos.ptr_ == tail_ || pos.ptr_ == nullptr) {
            throw std::exception();
        }
        --size_;
        node* tmp = pos.ptr_->nxt_;
        delete erase(pos.ptr_);
        return iterator(tmp);
    }

    /**
     * adds an element to the end
     */
    void push_back(const T& value) {
        ++size_;
        insert(tail_, new node(value, nullptr, nullptr));
    }

    void emplace_back(T&& value) {
        ++size_;
        insert(tail_, new node(std::move(value), nullptr, nullptr));
    }

    /**
     * removes the last element
     * throw when the container is empty.
     */
    void pop_back() {
        if (size_ == 0) {
            throw std::exception();
        }
        --size_;
        delete erase(tail_->pre_);
    }

    /**
     * inserts an element to the beginning.
     */
    void push_front(const T& value) {
        ++size_;
        insert(head_->nxt_, new node(value, nullptr, nullptr));
    }

    /**
     * removes the first element.
     * throw when the container is empty.
     */
    void pop_front() {
        if (size_ == 0) {
            throw std::exception();
        }
        --size_;
        delete erase(head_->nxt_);
    }
    void splice(iterator pos, list<T>& other) {
        if (other.empty()) {
            return;  // Nothing to splice
        }

        // Splice all elements from `other` before `pos`
        node* first = other.head_->nxt_;
        node* last = other.tail_->pre_;

        // Disconnect the elements from `other`
        other.head_->nxt_ = other.tail_;
        other.tail_->pre_ = other.head_;
        other.size_ = 0;

        // Insert the range from `other` before `pos` in the current list
        first->pre_ = pos.ptr_->pre_;
        last->nxt_ = pos.ptr_;
        pos.ptr_->pre_->nxt_ = first;
        pos.ptr_->pre_ = last;

        size_ += other.size_;
        other.size_ = 0;  // `other` is now empty
    }
};

}  // namespace sjtu

#endif  // SJTU_LIST_HPP