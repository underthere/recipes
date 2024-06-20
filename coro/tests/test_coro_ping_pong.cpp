#include <iostream>

#include <async_simple/coro/Sleep.h>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

#include "../coro_io/io_context_pool.hpp"

auto pong() -> async_simple::coro::Lazy<void>;
auto pong() -> async_simple::coro::Lazy<void>;

auto ping() -> async_simple::coro::Lazy<void> {
    std::cout << "ping" << std::endl;
    co_await async_simple::coro::sleep(std::chrono::milliseconds(100));
    co_await pong();
}

auto pong() -> async_simple::coro::Lazy<void> {
    std::cout << "pong" << std::endl;
    co_await async_simple::coro::sleep(std::chrono::milliseconds(100));
    co_await ping();
}

int main() {
    async_simple::coro::syncAwait(ping().via(coro_io::get_global_executor()));
    return 0;
}
