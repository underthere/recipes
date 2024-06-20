//
// Created on 2024/6/20.
//

#pragma once

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <tl/expected.hpp>

namespace rtsp {
using tl::expected;
using tl::make_unexpected;

enum class Method { OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN };

auto method_to_string(Method method) -> std::string;

auto string_to_method(const std::string& method) -> std::optional<Method>;

struct Request {
    Method method;
    std::string url;
    std::string version;
    std::string user_agent;
    std::string transport;
    std::string session;
    int cseq;

    auto operator==(const Request& other) const -> bool;
};

struct Reply {
    std::string version;
    int status_code;
    int cseq;
    std::vector<Method> public_methods;
    std::chrono::system_clock::time_point date;
};

auto parse_request(const std::string& request)
    -> expected<Request, std::error_code>;

auto dump_request(const Request& request) -> std::string;

}  // namespace rtsp