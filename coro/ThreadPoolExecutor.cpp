#include "ThreadPoolExecutor.h"

namespace underthere {

inline constexpr int16_t kContextMask = 0x40000000;

ThreadPoolExecutor::ThreadPoolExecutor(size_t numThreads): _pool(numThreads, true) {

}
ThreadPoolExecutor::~ThreadPoolExecutor() {

}
bool ThreadPoolExecutor::schedule(async_simple::Executor::Func func) {
  return _pool.schedule(func).has_value();
}
bool ThreadPoolExecutor::currentThreadInExecutor() const {
  return _pool.currentId() != -1;
}
async_simple::ExecutorStat ThreadPoolExecutor::stat() const {
  return async_simple::ExecutorStat {};
}
size_t ThreadPoolExecutor::currentContextId() const {
  return Executor::currentContextId();
}
async_simple::Executor::Context ThreadPoolExecutor::checkout() {
  return reinterpret_cast<Context>(_pool.currentId() | kContextMask);
}
bool ThreadPoolExecutor::checkin(async_simple::Executor::Func func,
                                 async_simple::Executor::Context ctx,
                                 async_simple::ScheduleOptions opts) {
  int64_t id = reinterpret_cast<int64_t>(ctx);
  auto prompt = _pool.currentId() == (id & (~kContextMask)) && opts.prompt;
  if (prompt) {
    func();
    return true;
  }
  return _pool.schedule(func, id & (~kContextMask)).has_value();
}
bool ThreadPoolExecutor::checkin(async_simple::Executor::Func func, async_simple::Executor::Context ctx) {
  return Executor::checkin(func, ctx);
}
async_simple::IOExecutor *ThreadPoolExecutor::getIOExecutor() {
  return nullptr;
}
void ThreadPoolExecutor::schedule(async_simple::Executor::Func func, async_simple::Executor::Duration dur) {
  Executor::schedule(func, dur);
}

} // underthere