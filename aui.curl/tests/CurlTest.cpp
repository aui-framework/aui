//
// Created by Alex2772 on 2/4/2022.
//

#include <gtest/gtest.h>
#include "AUI/Curl/ACurl.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Curl/AWebsocket.h"

TEST(CurlTest, ToByteBuffer) {
    AByteBuffer buffer = ACurl::Builder("https://github.com").toByteBuffer();
    ASSERT_TRUE(AString::fromUtf8(buffer).contains("DOCTYPE"));
}

TEST(CurlTest, ToStream) {
    {
        ACurl a(ACurl::Builder("https://github.com").withOutputStream(_new<AFileOutputStream>("temp.html")));
        a.run();
    }
    auto buffer = AByteBuffer::fromStream(AFileInputStream("temp.html"));
    ASSERT_TRUE(AString::fromUtf8(buffer).contains("DOCTYPE"));
}

TEST(CurlTest, Fail) {
    ACurl a(ACurl::Builder("https://snonejklgnjkwernjkg3ernjknerjk.sfjgsjknsdgjknsdgjknjksdgnjkzdg").withOutputStream(_new<AFileOutputStream>("temp.html")));
    EXPECT_THROW(a.run(), ACurl::Exception);
}

TEST(CurlTest, WebSocket) {
    auto ws = _new<AWebsocket>("wss://ws.postman-echo.com/raw");
    unsigned c = 0;
    AObject::connect(ws->connected, ws, [&] {
        ws->write("hello", 5);

        AObject::connect(ws->received, ws, [&](AByteBufferView data) {
            EXPECT_EQ(AString::fromUtf8(data), "hello");
            AObject::disconnect();
            AObject::connect(ws->received, ws, [&](AByteBufferView data) {
                EXPECT_EQ(AString::fromUtf8(data), "hello");
                AObject::disconnect();
                ws->close();
                c++;
            });
        });
    });
    ws->run();
    EXPECT_EQ(c, 2) << "not enough payloads received";
}
