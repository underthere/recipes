#pragma once

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <expected>
#include "expected_wrapper.h"

namespace underthere {

template<typename T> requires std::is_move_assignable_v<T>
class Queue {
 public:
  void push(T &&item) {
    {
      std::scoped_lock guard(_mutex);
      _queue.push(std::move(item));
    }
    _cond.notify_one();
  }

  bool try_push(const T &item) {
    {
      std::unique_lock lock(_mutex, std::try_to_lock);
      if (!lock)
        return false;
      _queue.push(item);
    }
    _cond.notify_one();
    return true;
  }

  bool pop(T &item) {
    std::unique_lock lock(_mutex);
    _cond.wait(lock, [&]() { return !_queue.empty() || _stop; });
    if (_queue.empty())
      return false;
    item = std::move(_queue.front());
    _queue.pop();
    return true;
  }

  bool try_pop(T &item) {
    std::unique_lock lock(_mutex, std::try_to_lock);
    if (!lock || _queue.empty())
      return false;

    item = std::move(_queue.front());
    _queue.pop();
    return true;
  }

  // non-blocking pop an item, maybe pop failed.
  // predict is an extension pop condition, default is null.
  bool try_pop_if(T &item, bool (*predict)(T &) = nullptr) {
    std::unique_lock lock(_mutex, std::try_to_lock);
    if (!lock || _queue.empty())
      return false;

    if (predict && !predict(_queue.front())) {
      return false;
    }

    item = std::move(_queue.front());
    _queue.pop();
    return true;
  }

  std::size_t size() const {
    std::scoped_lock guard(_mutex);
    return _queue.size();
  }

  bool empty() const {
    std::scoped_lock guard(_mutex);
    return _queue.empty();
  }

  void stop() {
    {
      std::scoped_lock guard(_mutex);
      _stop = true;
    }
    _cond.notify_all();
  }

 private:
  std::queue<T> _queue;
  bool _stop = false;
  mutable std::mutex _mutex;
  std::condition_variable _cond;
};

class ThreadPool {
  using Func = std::function<void()>;
  auto current() const -> std::tuple<size_t, ThreadPool *> &;
  struct Task {
    explicit Task(Func &&func, bool canSteal = false)
        : func(std::move(func)), canSteal(canSteal) {
    }
    Task() = default;

    Func func;
    bool canSteal{false};
  };

 public:
  explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency(), bool enableWorkSteal = false);

  ~ThreadPool();

  Expected<void, std::error_code> schedule(Func func, int32_t id = -1);
  int32_t currentId() const;
  size_t taskCount() const;
  int32_t threadCount() const;

 private:

  int32_t threadNum_;
  std::vector<std::thread> threads_;
  std::vector<Queue<Task>> queue_;
  std::atomic<bool> running_{false};
  bool enableWorkSteal_;
};
}