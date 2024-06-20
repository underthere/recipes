//
// Created on 2024/6/20.
//

#include "rtsp_protocol.h"
#include <algorithm>

std::string strip(const std::string& s) {
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c) || c == '\n' || c == '\r';
    });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) {
                   return std::isspace(c) || c == '\n' || c == '\r';
               }).base();

    return (start < end ? std::string(start, end) : std::string());
}

namespace rtsp {

auto Request::operator==(const Request& other) const -> bool {
    return method == other.method && url == other.url &&
           version == other.version && user_agent == other.user_agent &&
           cseq == other.cseq;
}

auto method_to_string(Method method) -> std::string {
    switch (method) {
        case Method::OPTIONS:
            return "OPTIONS";
        case Method::DESCRIBE:
            return "DESCRIBE";
        case Method::SETUP:
            return "SETUP";
        case Method::PLAY:
            return "PLAY";
        case Method::TEARDOWN:
            return "TEARDOWN";
    }
}

auto string_to_method(const std::string& method) -> std::optional<Method> {
    if (method == "OPTIONS") {
        return Method::OPTIONS;
    } else if (method == "DESCRIBE") {
        return Method::DESCRIBE;
    } else if (method == "SETUP") {
        return Method::SETUP;
    } else if (method == "PLAY") {
        return Method::PLAY;
    } else if (method == "TEARDOWN") {
        return Method::TEARDOWN;
    }
    return std::nullopt;
}

auto parse_request(const std::string& request)
    -> expected<Request, std::error_code> {
    rtsp::Request req;

    std::istringstream request_stream(request);
    std::string line;

    // Parse the first line
    if (std::getline(request_stream, line)) {
        std::istringstream line_stream(line);
        std::string method, url, version;

        if (std::getline(line_stream, method, ' ') &&
            std::getline(line_stream, url, ' ') &&
            std::getline(line_stream, version)) {
            auto method_opt = rtsp::string_to_method(method);
            if (method_opt.has_value()) {
                req.method = method_opt.value();
            }
            req.url = strip(url);
            req.version = strip(version);
        }
    }

    // Parse headers
    while (std::getline(request_stream, line)) {
        std::istringstream line_stream(line);
        std::string header_name, header_value;

        if (std::getline(line_stream, header_name, ':') &&
            std::getline(line_stream, header_value)) {
            header_value = strip(header_value);
            if (header_name == "CSeq") {
                req.cseq = std::stoi(header_value);
            }
            if (header_name == "User-Agent") {
                req.user_agent = header_value;
            }
        }
    }

    return req;
}

auto dump_request(const Request& request) -> std::string {
    std::ostringstream request_stream;
    request_stream << rtsp::method_to_string(request.method) << " "
                   << request.url << " " << request.version << "\r\n"
                   << "CSeq: " << request.cseq << "\r\n"
                   << "User-Agent: " << request.user_agent << "\r\n"
                   << "\r\n";
    return request_stream.str();
}

}  // namespace rtsp