#include <iostream>
#include <asio.hpp>


asio::awaitable<void> test() {
    std::cout << "test" << std::endl;
    co_return;
}

int main() {
    asio::io_context io_context {4};
    asio::co_spawn(io_context, test(), asio::detached);
    io_context.run();
    return 0;
}
