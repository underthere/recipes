#pragma once

#include <stddef.h>
#include <vector>
#include <optional>

#include "error_code.h"
#include "Awaitable.h"
#include "expected_wrapper.h"

#include "RingBuffer.h"

namespace underthere::coro {
namespace ds {

template <typename T>
class Queue {
 public:
  virtual ~Queue() = default;
  virtual auto push(T &&value, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<void, std::error_code>> = 0;
  virtual auto push(const T &value, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<void, std::error_code>> = 0;
  virtual auto pop(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<T, std::error_code>> = 0;
};


template<typename T>
class BlockQueue : public virtual Queue<T> {
 public:
  explicit BlockQueue(size_t n) : n_(n), buffer_(n) {}
  auto push(T &&value, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<void, std::error_code>> override {
    co_await lock_.coLock();
    cond_not_full_.wait(lock_, [this] { return !buffer_.full(); });
    buffer_.push(std::move(value));
    cond_not_empty_.notifyAll();
    co_return {};
  }
  auto push(const T &value, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<void, std::error_code>> override {
    co_await lock_.coLock();
    cond_not_full_.wait(lock_, [this] { return !buffer_.full(); });
    buffer_.push(value);
    cond_not_empty_.notifyAll();
    co_return {};
  }
  auto pop(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> Awaitable<Expected<T, std::error_code>> override {
    co_await lock_.coLock();
    cond_not_empty_.wait(lock_, [this] { return !buffer_.empty(); });
    auto value = buffer_.pop();
    cond_not_full_.notifyAll();
    co_return {};
  }
 private:
  size_t n_;
  RingBuffer<T> buffer_;
  CoMutex lock_;
  CoConditionVariable<CoMutex> cond_not_empty_;
  CoConditionVariable<CoMutex> cond_not_full_;
};

} // ds
} // underthere
