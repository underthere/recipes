#pragma once

#include "thirdparty/async_simple/async_simple/Executor.h"
#include "ThreadPool.hpp"

namespace underthere {
class ThreadPoolExecutor : public virtual async_simple::Executor {
 public:
  explicit ThreadPoolExecutor(size_t numThreads);
  ~ThreadPoolExecutor() override;
  bool schedule(Func func) override;
  bool currentThreadInExecutor() const override;
  async_simple::ExecutorStat stat() const override;
  size_t currentContextId() const override;
  Context checkout() override;
  bool checkin(Func func, Context ctx, async_simple::ScheduleOptions opts) override;
  bool checkin(Func func, Context ctx) override;
  async_simple::IOExecutor *getIOExecutor() override;
 protected:
  void schedule(Func func, Duration dur) override;

 private:
  ThreadPool _pool;
};

} // underthere