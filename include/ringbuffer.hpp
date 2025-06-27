#pragma once

#include <atomic>
#include <vector>
#include <cstddef>

template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : capacity_(capacity + 1), buffer_(capacity_), head_(0), tail_(0) {}

    bool push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t nextHead = increment(head);

        if (nextHead == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }

        buffer_[head] = item;
        head_.store(nextHead, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }

        item = buffer_[tail];
        tail_.store(increment(tail), std::memory_order_release);
        return true;
    }

    bool isEmpty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    bool isFull() const {
        return increment(head_.load(std::memory_order_acquire)) == tail_.load(std::memory_order_acquire);
    }

    size_t size() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (head + capacity_ - tail) % capacity_;
    }

private:
    size_t increment(size_t index) const {
        return (index + 1) % capacity_;
    }

    const size_t capacity_;
    std::vector<T> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

