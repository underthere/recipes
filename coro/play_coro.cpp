#include "Awaitable.h"

#include <async_simple/coro/SyncAwait.h>
#include "coro_io/io_context_pool.hpp"
#include <future>

Awaitable<int> async_main() {
  auto f = [](int a, int b) -> int {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return a + b;
  };
  co_return co_await sync_as_coro(f, 3, 4);
}

int main(int argc, char **argv) {
  auto executor = coro_io::get_global_executor();
  return async_simple::coro::syncAwait(async_main().via(executor));
}
