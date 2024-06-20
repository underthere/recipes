#pragma once

#include <type_traits>
#include "thirdparty/async_simple/async_simple/coro/Lazy.h"
#include "thirdparty/async_simple/async_simple/Promise.h"
#include "thirdparty/async_simple/async_simple/coro/FutureAwaiter.h"
#include "thirdparty/async_simple/async_simple/coro/SpinLock.h"
#include "thirdparty/async_simple/async_simple/coro/ConditionVariable.h"

#define Awaitable async_simple::coro::Lazy
#define CoMutex async_simple::coro::SpinLock
#define CoConditionVariable async_simple::coro::ConditionVariable

template <typename Func, typename... Args>
auto sync_as_coro(Func func, Args... args)
-> Awaitable<typename std::invoke_result_t<Func, Args...>> {
  using T = typename std::invoke_result_t<Func, Args...>;
  auto boundFunc = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
  auto promise = async_simple::Promise<T>();
  auto executor = co_await async_simple::CurrentExecutor();
  auto fut = promise.getFuture().via(executor);
  auto impl = [promise = std::move(promise), boundFunc = std::move(boundFunc)]() mutable {
    promise.setValue(boundFunc());
  };
  if (executor) {
    executor->schedule(impl);
  } else {
    std::thread(impl).detach();
  }
  co_return co_await std::move(fut);
}
