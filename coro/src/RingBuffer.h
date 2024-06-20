#pragma once

#include <stddef.h>
#include <vector>
#include <optional>

#include "error_code.h"
#include "expected_wrapper.h"

namespace underthere::ds {

template<typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t size) : capacity_(size), buffer_(capacity_), head_(0), tail_(0), full_(false) {}

  inline auto capacity() const -> size_t {
    return capacity_;
  }

  inline auto size() const -> size_t {
    if (full_) return capacity_;
    if (tail_ == head_) return 0;
    if (tail_ > head_) return tail_ - head_;
    return capacity_ - head_ + tail_;
  }

  inline auto empty() const -> bool {
    return size() == 0;
  }

  inline auto full() const -> bool {
    return full_;
  }

  inline auto push(const T &value) -> Expected<void, std::error_code> {
    if (full()) {
      return UnExpected(make_error_code(0));
    }
    buffer_[tail_] = value;
    tail_ = (tail_ + 1) % capacity_;
    full_ = tail_ == head_;
    return {};
  }

  inline auto pop() -> Expected<T, std::error_code> {
    if (empty()) {
      return UnExpected(make_error_code(0));
    }
    auto value = buffer_[head_];
    head_ = (head_ + 1) % capacity_;
    full_ = false;
    return value;
  }

 private:
  size_t capacity_;
  size_t head_;
  size_t tail_;
  bool full_;
  std::vector<T> buffer_;
};

} // ds
// underthere
