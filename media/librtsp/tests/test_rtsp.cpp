#include <gtest/gtest.h>

#include "rtsp_protocol.h"

using namespace rtsp;

TEST(RtspProtocol, test_method_conversion) {
    EXPECT_EQ(method_to_string(Method::OPTIONS), "OPTIONS");
    EXPECT_EQ(method_to_string(Method::DESCRIBE), "DESCRIBE");
    EXPECT_EQ(method_to_string(Method::SETUP), "SETUP");
    EXPECT_EQ(method_to_string(Method::PLAY), "PLAY");
    EXPECT_EQ(method_to_string(Method::TEARDOWN), "TEARDOWN");

    EXPECT_EQ(string_to_method("OPTIONS"), Method::OPTIONS);
    EXPECT_EQ(string_to_method("DESCRIBE"), Method::DESCRIBE);
    EXPECT_EQ(string_to_method("SETUP"), Method::SETUP);
    EXPECT_EQ(string_to_method("PLAY"), Method::PLAY);
    EXPECT_EQ(string_to_method("TEARDOWN"), Method::TEARDOWN);

    EXPECT_EQ(string_to_method("INVALID"), std::nullopt);
}

TEST(RtspProtocol, test_parse_request) {
    std::string request = "OPTIONS rtsp://example.com:554 RTSP/1.0\r\n"
                          "CSeq: 1\r\n"
                          "User-Agent: test\r\n"
                          "\r\n";
    auto req = parse_request(request);
    EXPECT_TRUE(req.has_value());
    EXPECT_EQ(req->method, Method::OPTIONS);
    EXPECT_EQ(req->url, "rtsp://example.com:554");
    EXPECT_EQ(req->version, "RTSP/1.0");
    EXPECT_EQ(req->user_agent, "test");
    EXPECT_EQ(req->cseq, 1);
}


TEST(RtspProtocol, test_dump_request) {
        Request req;
        req.method = Method::OPTIONS;
        req.url = "rtsp://example.com:554";
        req.version = "RTSP/1.0";
        req.user_agent = "test";
        req.cseq = 1;
        auto redump = parse_request(dump_request(req));
        EXPECT_TRUE(redump.has_value());
        EXPECT_EQ(redump.value(), req);
}