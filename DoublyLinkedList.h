// DoublyLinkedList.h - Task A1: generic doubly linked list (class template).
// The list knows nothing about library records: it stores any copyable type T.
// Record-specific rules (validation, duplicate ids) are supplied by the caller
// through predicates and functors, see ReturnRegister.h.
#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <cstddef>
#include <fstream>
#include <new>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class DoublyLinkedList {
public:
    DoublyLinkedList() : head_(nullptr), tail_(nullptr), count_(0) {}

    // Deep copy: every node is newly allocated, nothing is shared with 'other'.
    DoublyLinkedList(const DoublyLinkedList& other) : head_(nullptr), tail_(nullptr), count_(0) {
        try {
            for (Node* p = other.head_; p; p = p->next)
                if (!pushBack(p->data)) throw std::bad_alloc();
        } catch (...) {          // a destructor does NOT run if a constructor throws
            clear();
            throw;
        }
    }
    // Move: steal the other list's nodes.
    DoublyLinkedList(DoublyLinkedList&& o) noexcept : head_(o.head_), tail_(o.tail_), count_(o.count_) {
        o.head_ = o.tail_ = nullptr;
        o.count_ = 0;
    }
    // Copy-and-swap: the by-value parameter is the copy; it frees our old nodes when it dies.
    // Self-assignment is safe.
    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept {
        swap(other);
        return *this;
    }
    ~DoublyLinkedList() { clear(); }

    // ---- insertion: return false (list unchanged) if memory cannot be allocated ----
    bool pushBack(const T& v) {
        Node* n = makeNode(v);
        if (!n) return false;
        n->prev = tail_;
        if (tail_) tail_->next = n; else head_ = n;
        tail_ = n;
        ++count_;
        return true;
    }
    bool pushFront(const T& v) {
        Node* n = makeNode(v);
        if (!n) return false;
        n->next = head_;
        if (head_) head_->prev = n; else tail_ = n;
        head_ = n;
        ++count_;
        return true;
    }

    // ---- removal: return false if the list is empty / nothing matches ----
    bool removeFirst(T* removed = nullptr) {
        if (!head_) return false;
        if (removed) *removed = head_->data;
        unlink(head_);
        return true;
    }
    bool removeLast(T* removed = nullptr) {
        if (!tail_) return false;
        if (removed) *removed = tail_->data;
        unlink(tail_);
        return true;
    }
    // Removes the first element for which pred(element) is true.
    template <typename Pred>
    bool removeIf(Pred pred, T* removed = nullptr) {
        for (Node* p = head_; p; p = p->next) {
            if (pred(p->data)) {
                if (removed) *removed = p->data;
                unlink(p);
                return true;
            }
        }
        return false;
    }

    // ---- query ----
    // Pointer to the first element for which pred(element) is true, or nullptr.
    template <typename Pred>
    const T* findIf(Pred pred) const {
        for (Node* p = head_; p; p = p->next)
            if (pred(p->data)) return &p->data;
        return nullptr;
    }
    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    void clear() {
        Node* p = head_;
        while (p) {
            Node* nx = p->next;
            delete p;
            p = nx;
        }
        head_ = tail_ = nullptr;
        count_ = 0;
    }

    // ---- traversal ----
    template <typename Fn> void forEach(Fn f) const {          // head -> tail
        for (Node* p = head_; p; p = p->next) f(p->data);
    }
    template <typename Fn> void forEachReverse(Fn f) const {   // tail -> head
        for (Node* p = tail_; p; p = p->prev) f(p->data);
    }

    // ---- file I/O (one record per line; '#' comment lines and blank lines are ignored) ----
    // parse(line, T& out)         -> true if the line is a valid record
    // accept(const T&, const list&) -> true if the record may be added (e.g. not a duplicate id)
    // Returns the number of records loaded, or -1 if the file cannot be opened (list untouched).
    // Bad lines are skipped and counted in *skipped. The list is replaced only at the end.
    template <typename Parse, typename Accept>
    int load(const std::string& path, Parse parse, Accept accept, int* skipped = nullptr) {
        std::ifstream in(path);
        if (!in) return -1;
        DoublyLinkedList temp;                 // build aside; commit only at the end
        int bad = 0;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            T rec;
            if (!parse(line, rec) || !accept(rec, temp) || !temp.pushBack(rec)) ++bad;
        }
        if (skipped) *skipped = bad;
        int n = static_cast<int>(temp.count_);
        swap(temp);
        return n;
    }
    // format(const T&) -> one line of text (no newline). 'header' is written first.
    template <typename Format>
    bool save(const std::string& path, Format format, const std::string& header = "") const {
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        if (!header.empty()) out << header << '\n';
        for (Node* p = head_; p; p = p->next) out << format(p->data) << '\n';
        out.flush();
        return static_cast<bool>(out);
    }

    // ---- bridge to the vector-based sorting code (A2) ----
    std::vector<T> toVector() const {
        std::vector<T> v;
        v.reserve(count_);
        for (Node* p = head_; p; p = p->next) v.push_back(p->data);
        return v;
    }
    bool replaceWith(const std::vector<T>& v) {   // list takes the vector's order; unchanged on failure
        DoublyLinkedList temp;
        for (const T& x : v)
            if (!temp.pushBack(x)) return false;
        swap(temp);
        return true;
    }

    // Test hook: checks that prev/next/head/tail/count are consistent.
    bool selfCheck() const {
        std::size_t n = 0;
        Node* last = nullptr;
        for (Node* p = head_; p; p = p->next) {
            if (p->prev != last) return false;
            last = p;
            ++n;
            if (n > count_) return false;
        }
        if (last != tail_ || n != count_) return false;
        if (!head_ && (tail_ || count_)) return false;
        if (head_ && head_->prev) return false;
        if (tail_ && tail_->next) return false;
        return true;
    }

private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& v) : data(v), prev(nullptr), next(nullptr) {}
    };
    Node* head_;
    Node* tail_;
    std::size_t count_;

    static Node* makeNode(const T& v) {
        try { return new (std::nothrow) Node(v); }
        catch (...) { return nullptr; }            // copying T itself may throw
    }
    void unlink(Node* n) {                         // detaches and deletes n (all 4 cases)
        if (n->prev) n->prev->next = n->next; else head_ = n->next;
        if (n->next) n->next->prev = n->prev; else tail_ = n->prev;
        delete n;
        --count_;
    }
    void swap(DoublyLinkedList& o) noexcept {
        std::swap(head_, o.head_);
        std::swap(tail_, o.tail_);
        std::swap(count_, o.count_);
    }
};

#endif // DOUBLY_LINKED_LIST_H
