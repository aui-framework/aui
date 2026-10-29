#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <AUI/Image/png/PngImageLoader.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Util/ARandom.h>

#include "FakeAgent.h"
#include "remote/FrameStream.h"

using namespace aui_remote;
using namespace aui_remote::test;
using namespace std::chrono_literals;

namespace {

AppInfo appOf(const FakeAgent& agent) {
    AppInfo app;
    app.pid = 1;
    app.port = agent.port();
    app.scheme = "wss";
    app.certSha256 = agent.certSha256();
    return app;
}

/// The frame is a PNG: AImage sniffs the format, and there is no JPEG encoder at hand.
AString imageBase64(glm::ivec2 size, bool noise = false) {
    AByteBuffer pixels;
    ARandom random;
    for (int i = 0; i < size.x * size.y * 4; ++i) {
        // noise does not compress: the frame then spans many TLS records
        const char byte = noise ? char(random.nextInt()) : char(i % 4 == 0 ? 0xff : 0);
        pixels.write(&byte, 1);
    }
    AImage image(std::move(pixels), glm::uvec2(size), APixelFormat::RGBA_BYTE);
    AByteBuffer buffer;
    PngImageLoader::save(buffer, image);
    return buffer.toBase64String();
}

AJson tree(AJson base, const char* text, bool add = false) {
    AJson::Array rootChildren { AJson("a") };
    AJson upsert = AJson::Object {};
    auto leaf = [](const char* t) {
        return AJson { { "type", "ALabel" }, { "text", AString(t) }, { "x", 1 }, { "y", 2 }, { "w", 3 }, { "h", 4 },
                       { "absX", 1 }, { "absY", 2 }, { "visible", true }, { "children", AJson(AJson::Array {}) } };
    };
    upsert["a"] = leaf(text);
    if (add) {
        rootChildren.push_back(AJson("b"));
        upsert["b"] = leaf("added");
    }
    upsert["w"] = AJson { { "type", "AWindow" }, { "x", 0 }, { "y", 0 }, { "w", 100 }, { "h", 50 }, { "absX", 0 },
                          { "absY", 0 }, { "visible", true }, { "children", AJson(std::move(rootChildren)) } };
    return AJson { { "base", std::move(base) }, { "root", "w" }, { "upsert", std::move(upsert) },
                   { "remove", AJson(AJson::Array {}) } };
}

AJson frame(int seq, AJson treeDiff, glm::ivec2 image = { 8, 4 }, bool noise = false) {
    return AJson { { "seq", seq },          { "width", image.x },   { "height", image.y },
                   { "windowWidth", 100 },  { "windowHeight", 50 }, { "jpeg", imageBase64(image, noise) },
                   { "tree", std::move(treeDiff) } };
}

/// Pumps the UI thread until the condition holds.
template <typename F>
bool waitFor(F&& condition, std::chrono::milliseconds timeout = 10s) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        AThread::processMessages();
        if (condition()) {
            return true;
        }
        std::this_thread::sleep_for(5ms);
    }
    return false;
}

}   // namespace

TEST(FrameStreamTest, SubscribesAndReceivesFramesWithAcks) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    AVector<FrameUpdate> frames;
    AVector<AString> failures;
    AObject::connect(stream->frameReceived, stream, [&](const FrameUpdate& f) { frames << f; });
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });

    stream->start(0.25f, 55);
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    EXPECT_NEAR(agent.lastSubscribeParams()["scale"].asNumber(), 0.25, 1e-6);
    EXPECT_EQ(agent.lastSubscribeParams()["quality"].asInt(), 55);

    agent.pushFrame(frame(1, tree(nullptr, "one")));
    ASSERT_TRUE(waitFor([&] { return frames.size() == 1 || !failures.empty(); }));
    ASSERT_TRUE(failures.empty()) << failures[0];
    EXPECT_EQ(frames[0].seq, 1);
    EXPECT_EQ(frames[0].windowSize, glm::ivec2(100, 50));
    ASSERT_TRUE(frames[0].image);
    EXPECT_EQ(frames[0].image->size(), glm::uvec2(8, 4));
    EXPECT_EQ(frames[0].tree->toJson()["children"][0]["text"].asString(), "one");
    ASSERT_TRUE(waitFor([&] { return agent.acks().size() == 1; }));
    EXPECT_EQ(agent.acks()[0], 1);

    // a gap in seq: 2..4 were dropped by the agent, 5 is a diff against the acked 1
    agent.pushFrame(frame(5, tree(1, "five", true)));
    ASSERT_TRUE(waitFor([&] { return frames.size() == 2; }));
    EXPECT_EQ(frames[1].seq, 5);
    EXPECT_EQ(frames[1].tree->toJson()["children"].asArray().size(), 2u);
    ASSERT_TRUE(waitFor([&] { return agent.acks().size() == 2; }));
    EXPECT_EQ(agent.acks()[1], 5);

    // the first update is immutable
    EXPECT_EQ(frames[0].tree->toJson()["children"].asArray().size(), 1u);
    EXPECT_TRUE(failures.empty());
    EXPECT_EQ(stream->lastErrorCode(), 0);
}

TEST(FrameStreamTest, DiffAgainstAnOlderFrameThanTheNewestApplied) {
    // the agent sends frame 3 before it has received the ack of frame 2, so it is built against frame 1
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    AVector<FrameUpdate> frames;
    AVector<AString> failures;
    AObject::connect(stream->frameReceived, stream, [&](const FrameUpdate& f) { frames << f; });
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));

    agent.pushFrame(frame(1, tree(nullptr, "one")));
    agent.pushFrame(frame(2, tree(1, "two", true)));
    agent.pushFrame(frame(3, tree(1, "three")));   // the second child is gone again, relative to frame 1
    ASSERT_TRUE(waitFor([&] { return frames.size() == 3 || !failures.empty(); }));
    ASSERT_TRUE(failures.empty()) << failures[0];
    EXPECT_EQ(frames[1].tree->toJson()["children"].asArray().size(), 2u);
    EXPECT_EQ(frames[2].tree->toJson()["children"].asArray().size(), 1u);
    EXPECT_EQ(frames[2].tree->toJson()["children"][0]["text"].asString(), "three");
    // don't tear the agent down while the acks are still on their way
    ASSERT_TRUE(waitFor([&] { return agent.acks().size() == 3; }));
}

TEST(FrameStreamTest, LargeFramesSpanningManyTlsRecords) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    AVector<FrameUpdate> frames;
    AObject::connect(stream->frameReceived, stream, [&](const FrameUpdate& f) { frames << f; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    for (int i = 1; i <= 3; ++i) {
        agent.pushFrame(frame(i, i == 1 ? tree(nullptr, "big") : tree(i - 1, "big"), { 150, 100 }, true));
    }
    ASSERT_TRUE(waitFor([&] { return frames.size() == 3; }));
    EXPECT_EQ(frames[2].image->size(), glm::uvec2(150, 100));
}

TEST(FrameStreamTest, CallsAreFireAndForget) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    stream->start();
    stream->call("input.pointer", AJson { { "type", "press" }, { "x", 5 }, { "y", 6 }, { "pointer", "touch" } });
    stream->call("input.pointer", AJson { { "type", "release" }, { "x", 5 }, { "y", 6 } });
    stream->call("input.key", AJson { { "key", "A" } });
    ASSERT_TRUE(waitFor([&] { return agent.inputCalls().size() == 3; }));
    auto calls = agent.inputCalls();
    EXPECT_EQ(calls[0].method, "input.pointer");
    EXPECT_EQ(calls[0].params["type"].asString(), "press");
    EXPECT_EQ(calls[0].params["pointer"].asString(), "touch");
    EXPECT_EQ(calls[1].params["type"].asString(), "release");
    EXPECT_EQ(calls[2].method, "input.key");
    EXPECT_EQ(agent.subscribeCalls(), 1);
}

TEST(FrameStreamTest, RequestReturnsTheResult) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    stream->start();
    auto future = stream->request("view.find", AJson { { "text", "ok" } });
    ASSERT_TRUE(waitFor([&] { return future.hasResult(); }));
    EXPECT_EQ((*future)["echo"]["text"].asString(), "ok");

    auto failing = stream->request("fail");
    ASSERT_TRUE(waitFor([&] { return failing.hasResult(); }));
    try {
        *failing;
        FAIL();
    } catch (const AInvocationTargetException& wrapper) {   // AFuture wraps what was supplied
        try {
            std::rethrow_exception(wrapper.causedBy());
        } catch (const RpcError& e) {
            EXPECT_EQ(e.code(), -32002);
        }
    }
}

TEST(FrameStreamTest, DesyncFailsTheStream) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    AVector<AString> failures;
    int frames = 0;
    AObject::connect(stream->frameReceived, stream, [&](const FrameUpdate&) { ++frames; });
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    agent.pushFrame(frame(1, tree(nullptr, "one")));
    ASSERT_TRUE(waitFor([&] { return frames == 1; }));
    agent.pushFrame(frame(9, tree(4, "bad")));   // built against 4, the client holds 1
    ASSERT_TRUE(waitFor([&] { return failures.size() == 1; }));
    EXPECT_TRUE(failures[0].contains("desync"));
    EXPECT_EQ(frames, 1);
    EXPECT_FALSE(stream->isStarted());
    EXPECT_EQ(stream->lastErrorCode(), rpc_code::CLIENT_ERROR);
}

TEST(FrameStreamTest, UnauthorizedExposesTheCode) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "wrong");
    AVector<AString> failures;
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return failures.size() == 1; }));
    EXPECT_EQ(stream->lastErrorCode(), rpc_code::UNAUTHORIZED);
    EXPECT_FALSE(stream->isStarted());
}

TEST(FrameStreamTest, LostConnectionFails) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    AVector<AString> failures;
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    auto pending = stream->request("window.list");   // answered; just to be sure the session works
    ASSERT_TRUE(waitFor([&] { return pending.hasResult(); }));
    agent.dropConnections();
    ASSERT_TRUE(waitFor([&] { return failures.size() == 1; }));
    EXPECT_EQ(stream->lastErrorCode(), rpc_code::CLIENT_ERROR);
}

TEST(FrameStreamTest, ConnectionToNothingFails) {
    AppInfo app;
    {
        FakeAgent agent("k");
        app = appOf(agent);
    }
    auto stream = _new<FrameStream>(app, "k");
    AVector<AString> failures;
    AObject::connect(stream->failed, stream, [&](const AString& e) { failures << e; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return failures.size() == 1; }));
}

TEST(FrameStreamTest, StopFailsPendingRequestsAndAllowsRestart) {
    FakeAgent agent("k");
    auto stream = _new<FrameStream>(appOf(agent), "k");
    int frames = 0;
    AObject::connect(stream->frameReceived, stream, [&](const FrameUpdate&) { ++frames; });
    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    stream->stop();
    EXPECT_FALSE(stream->isStarted());
    EXPECT_TRUE(waitFor([&] { return agent.subscribers() == 0; }));
    EXPECT_THROW(*stream->request("window.list"), AInvocationTargetException);

    stream->start();
    ASSERT_TRUE(waitFor([&] { return agent.subscribers() == 1; }));
    agent.pushFrame(frame(1, tree(nullptr, "again")));   // a fresh session: full tree again
    ASSERT_TRUE(waitFor([&] { return frames == 1; }));
}
