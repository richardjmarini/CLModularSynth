#pragma once

#include <atomic>
#include <vector>
#include <cstddef>

using namespace std;

template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : capacity_(capacity + 1), buffer_(capacity_), head_(0), tail_(0) {}

    bool push(const T &item) {
        size_t head = head_.load(memory_order_relaxed);
        size_t nextHead = increment(head);

        if (nextHead == tail_.load(memory_order_acquire)) {
            return false; // full
        }

        buffer_[head] = item;
        head_.store(nextHead, memory_order_release);
        return true;
    }

    bool pop(T& item) {

        size_t tail = tail_.load(memory_order_relaxed);

        if (tail == head_.load(memory_order_acquire)) {
            return false;
        }

        item = buffer_[tail];
        tail_.store(increment(tail), memory_order_release);

        return true;
    }

    bool isEmpty() const {
        return head_.load(memory_order_acquire) == tail_.load(memory_order_acquire);
    }

    bool isFull() const {
        return increment(head_.load(memory_order_acquire)) == tail_.load(memory_order_acquire);
    }

    size_t size() const {
        size_t head = head_.load(memory_order_acquire);
        size_t tail = tail_.load(memory_order_acquire);
        return (head + capacity_ - tail) % capacity_;
    }

    void copyFromTail(size_t offsetFromHead, T* dest, size_t count) {
        size_t safeHead = head_;
        size_t safeTail = tail_;
    
        size_t size;
        if (safeHead >= safeTail) {
            size = safeHead - safeTail;
        } else {
            size = capacity_ - (safeTail - safeHead);
        }
    
        // Check: do we have enough data to satisfy the request?
        if (offsetFromHead + count > size) {
            std::fill(dest, dest + count, 0);
            return;
        }
    
        // We want to copy `count` samples ending `offsetFromHead` samples before head
        size_t startIndex = (safeHead + capacity_ - offsetFromHead - count) % capacity_;
    
        for (size_t i = 0; i < count; ++i) {
            dest[i] = buffer_[(startIndex + i) % capacity_];
        }
    }

private:
    size_t increment(size_t index) const {
        return (index + 1) % capacity_;
    }

    const size_t capacity_;
    vector<T> buffer_;
    atomic<size_t> head_;
    atomic<size_t> tail_;
};

