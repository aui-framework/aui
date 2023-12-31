// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 2/4/2022.
//

#include <gtest/gtest.h>
#include "AUI/Curl/ACurl.h"
#include "AUI/Json/AJson.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Curl/AWebsocket.h"
#include "AUI/Thread/AEventLoop.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Curl/ACurlMulti.h"
#include <random>
#include <gmock/gmock.h>

TEST(CurlTest, ToByteBuffer) {
    AByteBuffer buffer = ACurl::Builder("https://github.com").runBlocking().body;
    ASSERT_TRUE(AString::fromUtf8(buffer).contains("DOCTYPE"));
}

TEST(CurlTest, Post1) {
    auto buffer = AJson::fromBuffer(ACurl::Builder("https://httpbin.org/post")
            .withMethod(ACurl::Method::POST)
            .withParams("hello=world").runBlocking().body);
    try {
        EXPECT_STREQ(buffer["form"]["hello"].asString().toStdString().c_str(), "world") << AJson::toString(buffer);
    } catch (...) {
        FAIL() << "Whoops! " << AJson::toString(buffer);
    }
}
TEST(CurlTest, Post2) {
    auto buffer = AJson::fromBuffer(ACurl::Builder("https://httpbin.org/post")
            .withMethod(ACurl::Method::POST)
            .withHeaders({
                "Content-Type: application/json; charset=utf-8"
            })
            .withBody("[\"hello\"]").runBlocking().body);
    try {
        EXPECT_STREQ(buffer["data"].asString().toStdString().c_str(), "[\"hello\"]") << AJson::toString(buffer);
    } catch (...) {
        FAIL() << "Whoops! " << AJson::toString(buffer);
    }
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
    ACurl a(ACurl::Builder("https://snonejklgnjkwernjkg3ernjknerjk.sfjgsjknsdgjknsdgjknjksdgnjkzdg")
                .withOutputStream(_new<AFileOutputStream>("temp.html"))
                .throwExceptionOnError(true));
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
            ws->write("world", 5);
            c++;
            AObject::connect(ws->received, ws, [&](AByteBufferView data) {
                EXPECT_EQ(AString::fromUtf8(data), "world");
                AObject::disconnect();
                ws->close();
                c++;
            });
        });
    });
    ws->run();
    EXPECT_EQ(c, 2) << "not enough payloads received";
}

TEST(CurlTest, WebSocketLong) {
    auto ws = _new<AWebsocket>("wss://ws.postman-echo.com/raw");


    constexpr auto PAYLOAD_SIZE = 60'000;
    std::string dataActual;
    dataActual.reserve(PAYLOAD_SIZE);
    std::default_random_engine re;
    for (auto i = 0; i < PAYLOAD_SIZE; ++i) {
        dataActual.push_back(std::uniform_int_distribution(int('a'), int('z'))(re));
    }

    std::string dataReceived;

    AObject::connect(ws->connected, ws, [&] {

        ws->write(dataActual.data(), dataActual.length());

        AObject::connect(ws->received, ws, [&](AByteBufferView payload) {
            dataReceived += std::string_view(payload.data(), payload.size());
            if (dataReceived.size() == PAYLOAD_SIZE) ws->close();
        });
    });
    ws->run();
    EXPECT_EQ(dataActual, dataReceived);
}


class Slave: public AObject {
public:
    MOCK_METHOD(void, acceptMessage, (const AString& message));

    void acceptMessageSlot(AByteBufferView data) {
        auto msg = AString::fromUtf8(data);
        acceptMessage(msg);
    }
};
