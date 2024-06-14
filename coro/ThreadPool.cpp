#include "ThreadPool.hpp"
#include <cstdlib>
#include <cassert>

#include "error_code.h"

namespace underthere {
ThreadPool::ThreadPool(size_t numThreads, bool enableWorkSteal)
    : threadNum_(numThreads), queue_(numThreads), running_(true), enableWorkSteal_(enableWorkSteal) {
  auto worker = [this](size_t id) {
    auto &[pid, pool] = current();
    pid = id;
    pool = this;
    while (true) {
      Task task{};
      if (enableWorkSteal_) {
        for (size_t i = 0; i < threadNum_ * 2; ++i) {
          if (queue_.at(i % threadNum_).try_pop_if(task,
                                                   [](auto &t) { return t.canSteal; })) {
            break;
          }
        }
      }
      if (!task.func && !queue_.at(id).pop(task)) {
        if (!running_) {
          break;
        } else {
          continue;
        }
      }
      if (task.func) task.func();
    }

  };

  threads_.reserve(threadNum_);
  for (size_t i = 0; i < threadNum_; ++i) {
    threads_.emplace_back(worker, i);
  }
}
ThreadPool::~ThreadPool() {
  running_ = false;
  for (auto &queue : queue_) {
    queue.stop();
  }
  for (auto &thread : threads_) {
    thread.join();
  }
}
Expected<void, std::error_code> ThreadPool::schedule(ThreadPool::Func func, int32_t id) {
  if (!running_) {
    return std::unexpected(make_error_code(errc::UnknownError));
  }
  if (func == nullptr) {
    return std::unexpected(make_error_code(errc::UnknownError));
  }
  if (id == -1) {
    if (enableWorkSteal_) {
      Task task(std::move(func), true);
      for (size_t i = 0; i < threadNum_ * 2; ++i) {
        if (queue_.at(i % threadNum_).try_push(task)) {
          return {};
        }
      }
    }
    id = rand() % threadNum_;
    queue_.at(id).push(Task(std::move(func), enableWorkSteal_));
  } else {
    assert(id < threadNum_);
    queue_.at(id).push(Task(std::move(func), false));
  }
  return {};
}

inline auto ThreadPool::current() const -> std::tuple<size_t, ThreadPool *> & {
  static thread_local std::tuple<size_t, ThreadPool *> data(-1, nullptr);
  return data;
}

auto ThreadPool::currentId() const -> int32_t {
  auto &[id, pool] = current();
  return this == pool ? id : -1;
}

inline auto ThreadPool::taskCount() const -> size_t {
  size_t count = 0;
  for (auto &queue : queue_) {
    count += queue.size();
  }
  return count;
}

inline auto ThreadPool::threadCount() const -> int32_t {
  return threadNum_;
}

}  // namespace underthere