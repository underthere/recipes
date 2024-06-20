//
// Created on 2024/6/20.
//

#include <array>
#include <asio.hpp>
#include <format>
#include <iostream>

#include <tl/expected.hpp>

using tl::expected;
using tl::make_unexpected;

using namespace asio;

 // namespace rtsp

std::string strip(const std::string& s) {
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c) || c == '\n' || c == '\r';
    });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) {
                   return std::isspace(c) || c == '\n' || c == '\r';
               }).base();

    return (start < end ? std::string(start, end) : std::string());
}

auto echo(ip::tcp::socket socket) -> awaitable<void> {
    std::array<char, 1024> data{};
    while (true) {
        std::cout << "try to read\n";
        size_t n = co_await socket.async_read_some(buffer(data), use_awaitable);
        std::cout << "read done\n";

        std::cout << std::format("Received {} bytes: {}", n,
                                 strip(std::string(data.data(), n)));
        std::flush(std::cout);
        co_await async_write(socket, buffer(data, n), use_awaitable);
    }
}

auto async_main(io_context& ctx) -> awaitable<void> {
    ip::tcp::acceptor acceptor(ctx, ip::tcp::endpoint(ip::tcp::v4(), 8808));

    while (true) {
        ip::tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        std::cout << std::format("New connection from {}:{}\n",
                                 socket.remote_endpoint().address().to_string(),
                                 socket.remote_endpoint().port());
        auto coro = co_spawn(acceptor.get_executor(), echo(std::move(socket)), use_awaitable);
    }
}

int main() {
    io_context ctx;
    co_spawn(ctx, async_main(ctx), asio::detached);
    auto ret = ctx.run();
    return 0;
}