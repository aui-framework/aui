#include <gmock/gmock.h>
#include <random>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/AString.h"
#include "AUI/Curl/AWebsocket.h"
#include "AUI/Util/kAUI.h"
#include "gmock/gmock.h"

TEST(WebsocketTest, Long) {
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

namespace {
    class Receiver: public AObject {
    public:
        MOCK_METHOD(void, receive, (AByteBuffer payload));
    };
}

static const auto PAYLOAD = AByteBuffer::fromHexString("817e00a67b2265223a227472616465222c2245223a313731363330343334303036332c2273223a2242544355534454222c2274223a333630383135313033382c2270223a2237303230392e3630303030303030222c2271223a22302e3030303230303030222c2262223a32373333373738313236352c2261223a32373333373738313538352c2254223a313731363330343334303036332c226d223a747275652c224d223a747275657d817e00a67b2265223a227472616465222c2245223a313731363330343334303036332c2273223a2242544355534454222c2274223a333630383135313033392c2270223a2237303230392e3630303030303030222c2271223a22302e3032393335303030222c2262223a32373333373738313335392c2261223a32373333373738313538352c2254223a313731363330343334303036332c226d223a747275652c224d223a747275657d817e00a67b2265223a227472616465222c2245223a313731363330343334303036332c2273223a2242544355534454222c2274223a333630383135313034302c2270223a2237303230392e3630303030303030222c2271223a22302e3030323439303030222c2262223a32373333373738313336352c2261223a32373333373738313538352c2254223a313731363330343334303036332c226d223a747275652c224d223a747275657d");

TEST(WebsocketTest, Receive1) {
    auto r = _new<Receiver>();
    auto s = aui::ptr::manage_shared(new AWebsocket);
    AObject::connect(s->received, AUI_SLOT(r)::receive);


    testing::InSequence sequence;
    EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151038,"p":"70209.60000000","q":"0.00020000","b":27337781265,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
    EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151039,"p":"70209.60000000","q":"0.02935000","b":27337781359,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
    EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151040,"p":"70209.60000000","q":"0.00249000","b":27337781365,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
    EXPECT_EQ(s->onDataReceived(PAYLOAD), PAYLOAD.size());
}

TEST(WebsocketTest, Receive2) {

    for (int i = 1; i < PAYLOAD.size() - 1; ++i) {
        // this test slices the payload
        auto r = _new<Receiver>();
        auto s = aui::ptr::manage_shared(new AWebsocket);
        AObject::connect(s->received, AUI_SLOT(r)::receive);


        testing::InSequence sequence;
        EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151038,"p":"70209.60000000","q":"0.00020000","b":27337781265,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
        EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151039,"p":"70209.60000000","q":"0.02935000","b":27337781359,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
        EXPECT_CALL(*r, receive(AByteBuffer::fromString(R"({"e":"trade","E":1716304340063,"s":"BTCUSDT","t":3608151040,"p":"70209.60000000","q":"0.00249000","b":27337781365,"a":27337781585,"T":1716304340063,"m":true,"M":true})")));
        auto p1 = PAYLOAD.slice(0, i);
        auto p2 = PAYLOAD.slice(i);
        EXPECT_EQ(s->onDataReceived(p1), p1.size());
        EXPECT_EQ(s->onDataReceived(p2), p2.size());
    }
}