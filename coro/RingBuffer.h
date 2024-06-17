#pragma once

#include <stddef.h>
#include <vector>

#include "error_code.h"
#include "expected_wrapper.h"

namespace underthere {
namespace ds {

template<typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t size) : capacity_(size), buffer_(capacity_) {}

  inline auto capacity() const -> size_t {
    return capacity_;
  }

  inline auto size() const -> size_t {
    return (tail_ - head_ + capacity_) % capacity_;
  }

  inline auto empty() const -> bool {
    return head_ == (tail_ + 1) % capacity_;
  }

  inline auto full() const -> bool {
    return size() == capacity_;
  }

  inline auto push(const T &value) -> Expected<void, std::error_code> {
    if (full()) {
      return UnExpected(make_error_code(0));
    }
    buffer_[tail_] = value;
    tail_ = (tail_ + 1) % capacity_;
    return {};
  }

  inline auto pop() -> Expected<T, std::error_code> {
    if (empty()) {
      return UnExpected(make_error_code(0));
    }
    T value = buffer_[head_];
    head_ = (head_ + 1) % capacity_;
    return value;
  }

 private:
  size_t capacity_;
  size_t head_;
  size_t tail_;
  std::vector<T> buffer_;
};

} // ds
} // underthere
