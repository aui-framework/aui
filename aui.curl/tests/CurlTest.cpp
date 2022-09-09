//
// Created by Alex2772 on 2/4/2022.
//

#include <gtest/gtest.h>
#include "AUI/Curl/ACurl.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Curl/AWebsocket.h"
#include "AUI/Thread/AEventLoop.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Curl/ACurlMulti.h"
#include <random>
#include <gmock/gmock.h>

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

_<AWebsocket> makeWebsocket(std::string message, const _<Slave>& receiver) {
    auto ws = _new<AWebsocket>("wss://ws.postman-echo.com/raw");

    AObject::connect(ws->connected, ws, [message = std::move(message), ws = ws.get()] {
        ws->write(message.data(), message.size());
    });

    AObject::connect(ws->received, slot(receiver)::acceptMessageSlot);
    AObject::connect(ws->received, ws, [ws = ws.get()] {
        ws->close();
    });
    return ws;
}


TEST(CurlTest, WebSocketMulti) {
    auto receiver1 = _new<Slave>();
    EXPECT_CALL(*receiver1, acceptMessage("hello"_as));

    auto receiver2 = _new<Slave>();
    EXPECT_CALL(*receiver2, acceptMessage("world"_as));

    ACurlMulti::global() << makeWebsocket("hello", receiver1);
    ACurlMulti::global() << makeWebsocket("world", receiver2);

    AEventLoop l;
    IEventLoop::Handle h(&l);
    AThread::processMessages();
    while (!ACurlMulti::global().curls().empty()) {
        l.iteration();
    }
    AThread::processMessages(); // avoid possible data race
}
