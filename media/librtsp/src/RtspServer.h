
#pragma once

#include <asio.hpp>
#include "rtsp_protocol.h"

namespace rtsp {

class IRtspHandler {
public:
    virtual auto process_request(const Request& request)
        -> asio::awaitable<expected<Reply, std::error_code>> = 0;
};

class IConnection {
public:
    virtual auto close() -> asio::awaitable<void> = 0;

};

class IRtspConnection: public virtual IConnection {
public:
    virtual auto read_request()
        -> asio::awaitable<expected<Request, std::error_code>> = 0;
    virtual auto write_reply(const Reply& reply) -> asio::awaitable<void> = 0;
};

class IMediaConnection: public virtual IConnection {
public:
    virtual auto read_frame()
        -> asio::awaitable<expected<std::string, std::error_code>> = 0;
    virtual auto write_frame(const std::string& frame)
        -> asio::awaitable<void> = 0;
    virtual auto close() -> asio::awaitable<void> = 0;
};

class IRtspServer {};

}  // namespace rtsp
