#include <iostream>

#include <async_simple/coro/Sleep.h>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

#include <async_simple/uthread/Async.h>

#include "../coro_io/io_context_pool.hpp"

using namespace async_simple;

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
    auto ex = coro_io::get_global_executor();
    uthread::async<uthread::Launch::Current>([](){
        std::cout << "start" << std::endl;
    }, ex);
    return 0;
}
