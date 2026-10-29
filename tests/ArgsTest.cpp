#include <gtest/gtest.h>

#include "cli/Args.h"
#include "cli/Cli.h"

using namespace aui_remote;
using namespace aui_remote::cli;

namespace {
ParsedArgs parse(std::initializer_list<const char*> args) {
    AStringVector v;
    for (auto a : args) {
        v << a;
    }
    return ParsedArgs::parse(v);
}

CommandRequest request(std::initializer_list<const char*> args) {
    auto r = buildRequest(parse(args));
    EXPECT_TRUE(r.hasValue());
    return *r;
}
}   // namespace

TEST(ArgsTest, CommandPositionalAndOptions) {
    auto a = parse({ "click", "v1:5", "--button", "right", "--mod", "ctrl", "--mod", "shift" });
    EXPECT_EQ(a.command, "click");
    ASSERT_EQ(a.positional.size(), 1u);
    EXPECT_EQ(a.positional[0], "v1:5");
    EXPECT_EQ(a.get("button"), "right");
    EXPECT_EQ(a.getAll("mod"), (AStringVector { "ctrl", "shift" }));
}

TEST(ArgsTest, FlagsTakeNoValue) {
    auto a = parse({ "find", "--interactive", "--type", "Button" });
    EXPECT_TRUE(a.has("interactive"));
    EXPECT_EQ(a.get("type"), "Button");
    EXPECT_TRUE(a.positional.empty());
}

TEST(ArgsTest, EqualsSyntaxAndNegativeNumbers) {
    auto a = parse({ "scroll", "10", "20", "--dy=-120", "--dx", "-5" });
    EXPECT_EQ(a.positional.size(), 2u);
    EXPECT_EQ(a.getInt("dy"), -120);
    EXPECT_EQ(a.getInt("dx"), -5);
}

TEST(ArgsTest, RestAfterDoubleDash) {
    auto a = parse({ "launch", "--cwd", "/tmp", "--env", "A=1", "--env", "B=2", "--", "./app", "--flag", "x" });
    EXPECT_EQ(a.get("cwd"), "/tmp");
    EXPECT_EQ(a.getAll("env"), (AStringVector { "A=1", "B=2" }));
    EXPECT_EQ(a.rest, (AStringVector { "./app", "--flag", "x" }));
}

TEST(ArgsTest, OptionWithoutValueIsAnError) {
    EXPECT_THROW(parse({ "tree", "--depth" }), ArgsError);
}

TEST(ArgsTest, TreeRequest) {
    auto r = request({ "tree", "--depth", "3", "--inspector", "--ref", "v2:7" });
    EXPECT_EQ(r.method, "view.tree");
    EXPECT_EQ(r.params["depth"].asInt(), 3);
    EXPECT_EQ(r.params["ref"].asString(), "v2:7");
    EXPECT_EQ(r.params["include"][0].asString(), "inspector");
}

TEST(ArgsTest, FindRequest) {
    auto r = request({ "find", "--type", "Button", "--text", "OK", "--ass", "primary", "--interactive",
                       "--include-invisible" });
    EXPECT_EQ(r.method, "view.find");
    EXPECT_EQ(r.params["type"].asString(), "Button");
    EXPECT_EQ(r.params["text"].asString(), "OK");
    EXPECT_EQ(r.params["assName"].asString(), "primary");
    EXPECT_TRUE(r.params["interactive"].asBool());
    EXPECT_TRUE(r.params["includeInvisible"].asBool());
}

TEST(ArgsTest, ClickByRefAndByPoint) {
    auto byRef = request({ "click", "v1:3" });
    EXPECT_EQ(byRef.method, "input.pointer");
    EXPECT_EQ(byRef.params["action"].asString(), "click");
    EXPECT_EQ(byRef.params["ref"].asString(), "v1:3");
    EXPECT_FALSE(byRef.params.contains("x"));

    auto byPoint = request({ "click", "10", "20" });
    EXPECT_EQ(byPoint.params["x"].asInt(), 10);
    EXPECT_EQ(byPoint.params["y"].asInt(), 20);
    EXPECT_FALSE(byPoint.params.contains("ref"));
}

TEST(ArgsTest, DoubleClickMapsToAgentAction) {
    EXPECT_EQ(request({ "dblclick", "v1:3" }).params["action"].asString(), "doubleClick");
}

TEST(ArgsTest, DragAndScroll) {
    auto drag = request({ "drag", "10", "20", "v1:9" });
    EXPECT_EQ(drag.params["action"].asString(), "drag");
    EXPECT_EQ(drag.params["x"].asInt(), 10);
    EXPECT_EQ(drag.params["toRef"].asString(), "v1:9");

    auto drag2 = request({ "drag", "v1:1", "30", "40" });
    EXPECT_EQ(drag2.params["ref"].asString(), "v1:1");
    EXPECT_EQ(drag2.params["toX"].asInt(), 30);
    EXPECT_EQ(drag2.params["toY"].asInt(), 40);

    auto scroll = request({ "scroll", "v1:2", "--dy", "-120" });
    EXPECT_EQ(scroll.params["dy"].asInt(), -120);
    EXPECT_EQ(scroll.params["dx"].asInt(), 0);
}

TEST(ArgsTest, PointerIsLeftToTheApplicationUnlessChosen) {
    EXPECT_FALSE(request({ "click", "v1:1" }).params.contains("pointer"));
    EXPECT_EQ(request({ "click", "v1:1", "--pointer", "finger" }).params["pointer"].asString(), "finger");
    EXPECT_EQ(request({ "drag", "1", "2", "3", "4", "--pointer", "mouse" }).params["pointer"].asString(), "mouse");
    EXPECT_THROW(buildRequest(parse({ "click", "v1:1", "--pointer", "pen" })), ArgsError);
}

TEST(ArgsTest, TouchGestures) {
    auto longPress = request({ "long-press", "v1:1", "--duration", "40000" });
    EXPECT_EQ(longPress.method, "input.pointer");
    EXPECT_EQ(longPress.params["action"].asString(), "longPress");
    EXPECT_EQ(longPress.params["durationMs"].asInt(), 40000);
    EXPECT_GT(longPress.timeout, std::chrono::milliseconds(40000));

    auto fling = request({ "fling", "10", "20", "--dy", "-300", "--velocity", "2500.5" });
    EXPECT_EQ(fling.params["action"].asString(), "fling");
    EXPECT_EQ(fling.params["x"].asInt(), 10);
    EXPECT_EQ(fling.params["dy"].asInt(), -300);
    EXPECT_FALSE(fling.params.contains("dx"));
    EXPECT_DOUBLE_EQ(fling.params["velocity"].asNumber(), 2500.5);
    EXPECT_THROW(buildRequest(parse({ "fling", "v1:1" })), ArgsError);

    auto pinch = request({ "pinch", "v1:3", "--from", "50", "--to", "200", "--window", "v1:1" });
    EXPECT_EQ(pinch.method, "input.touch");
    EXPECT_EQ(pinch.params["gesture"].asString(), "pinch");
    EXPECT_EQ(pinch.params["ref"].asString(), "v1:3");
    EXPECT_EQ(pinch.params["to"].asInt(), 200);
    EXPECT_EQ(pinch.params["windowId"].asString(), "v1:1");
    EXPECT_THROW(buildRequest(parse({ "pinch", "v1:3", "--from", "50" })), ArgsError);

    auto rotate = request({ "rotate", "100", "100", "--degrees", "-45", "--duration", "500" });
    EXPECT_EQ(rotate.params["gesture"].asString(), "rotate");
    EXPECT_EQ(rotate.params["degrees"].asInt(), -45);
    EXPECT_EQ(rotate.params["durationMs"].asInt(), 500);
    EXPECT_THROW(buildRequest(parse({ "rotate", "100", "100" })), ArgsError);

    auto touch = request({ "touch", "--fingers", R"([{"points":[{"x":1,"y":2,"t":0},{"x":5,"y":2,"t":20000}]}])" });
    EXPECT_EQ(touch.method, "input.touch");
    EXPECT_EQ(touch.params["fingers"][0]["points"][1]["x"].asInt(), 5);
    EXPECT_GT(touch.timeout, std::chrono::milliseconds(20000));
    EXPECT_THROW(buildRequest(parse({ "touch" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "touch", "--fingers", "{oops" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "touch", "--fingers", "{}" })), ArgsError);
}

TEST(ArgsTest, PointNeedsBothCoordinates) {
    EXPECT_THROW(buildRequest(parse({ "click", "10" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "click" })), ArgsError);
}

TEST(ArgsTest, TypeAndKey) {
    auto type = request({ "type", "hello world", "--ref", "v1:4" });
    EXPECT_EQ(type.method, "input.type");
    EXPECT_EQ(type.params["text"].asString(), "hello world");
    EXPECT_EQ(type.params["ref"].asString(), "v1:4");

    auto key = request({ "key", "Enter", "--mod", "ctrl" });
    EXPECT_EQ(key.method, "input.key");
    EXPECT_EQ(key.params["key"].asString(), "Enter");
    EXPECT_EQ(key.params["modifiers"][0].asString(), "ctrl");
}

TEST(ArgsTest, WaitCommandsExtendClientTimeout) {
    auto idle = request({ "wait-idle", "--timeout", "20000", "--quiet", "100" });
    EXPECT_EQ(idle.method, "wait.idle");
    EXPECT_EQ(idle.params["timeoutMs"].asInt(), 20000);
    EXPECT_EQ(idle.params["quietMs"].asInt(), 100);
    EXPECT_GT(idle.timeout, std::chrono::milliseconds(20000));

    auto view = request({ "wait-view", "--text", "Done", "--state", "gone" });
    EXPECT_EQ(view.method, "wait.view");
    EXPECT_EQ(view.params["state"].asString(), "gone");
    EXPECT_EQ(request({ "wait-view", "--text", "x" }).params["state"].asString(), "visible");
    EXPECT_THROW(buildRequest(parse({ "wait-view", "--state", "blinking" })), ArgsError);
}

TEST(ArgsTest, ShotLogsCallHit) {
    auto shot = request({ "screenshot", "--ref", "v1:2", "--padding", "8", "--window", "v1:1" });
    EXPECT_EQ(shot.method, "screenshot");
    EXPECT_EQ(shot.params["padding"].asInt(), 8);
    EXPECT_EQ(shot.params["windowId"].asString(), "v1:1");

    auto logs = request({ "logs", "--since", "5", "--level", "warn", "--limit", "10" });
    EXPECT_EQ(logs.method, "diag.log");
    EXPECT_EQ(logs.params["since"].asInt(), 5);
    EXPECT_EQ(logs.params["level"].asString(), "warn");

    auto call = request({ "call", "openDocument", "--args", R"({"path":"/tmp/a"})" });
    EXPECT_EQ(call.method, "fn.call");
    EXPECT_EQ(call.params["name"].asString(), "openDocument");
    EXPECT_EQ(call.params["arguments"]["path"].asString(), "/tmp/a");
    EXPECT_THROW(buildRequest(parse({ "call", "f", "--args", "{oops" })), ArgsError);

    auto hit = request({ "hit", "5", "6" });
    EXPECT_EQ(hit.method, "view.hitTest");
    EXPECT_EQ(hit.params["x"].asInt(), 5);
}

TEST(ArgsTest, Resize) {
    auto resize = request({ "resize", "800", "600", "--window", "v1:1" });
    EXPECT_EQ(resize.method, "window.resize");
    EXPECT_EQ(resize.params["width"].asInt(), 800);
    EXPECT_EQ(resize.params["height"].asInt(), 600);
    EXPECT_EQ(resize.params["windowId"].asString(), "v1:1");
    EXPECT_GT(request({ "resize", "1", "1", "--timeout", "20000" }).timeout, std::chrono::milliseconds(20000));
    EXPECT_THROW(buildRequest(parse({ "resize", "800" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "resize", "800", "0" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "resize", "wide", "600" })), ArgsError);
}

TEST(ArgsTest, Scale) {
    auto scale = request({ "scale", "1.5", "--window", "v1:1" });
    EXPECT_EQ(scale.method, "window.setScale");
    EXPECT_DOUBLE_EQ(scale.params["factor"].asNumber(), 1.5);
    EXPECT_EQ(scale.params["windowId"].asString(), "v1:1");
    EXPECT_TRUE(request({ "scale", "reset" }).params["reset"].asBool());
    EXPECT_THROW(buildRequest(parse({ "scale" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "scale", "big" })), ArgsError);
    EXPECT_THROW(buildRequest(parse({ "scale", "0" })), ArgsError);
}

TEST(ArgsTest, ScrollIntoView) {
    auto r = request({ "scroll-into-view", "v1:7" });
    EXPECT_EQ(r.method, "view.scrollIntoView");
    EXPECT_EQ(r.params["ref"].asString(), "v1:7");
    EXPECT_THROW(buildRequest(parse({ "scroll-into-view" })), ArgsError);
}

TEST(ArgsTest, LocalCommandsAreNotAgentCalls) {
    for (auto c : { "list", "pair", "launch", "stop", "compare", "nonsense" }) {
        EXPECT_FALSE(buildRequest(parse({ c })).hasValue()) << c;
    }
}

namespace {
AppInfo app(std::uint32_t pid) {
    AppInfo a;
    a.pid = pid;
    a.port = std::uint16_t(40000 + pid);
    return a;
}
}   // namespace

TEST(ArgsTest, SelectAppPicksTheOnlyOne) {
    EXPECT_EQ(selectApp(parse({ "tree" }), { app(10) }).pid, 10u);
}

TEST(ArgsTest, SelectAppByPid) {
    EXPECT_EQ(selectApp(parse({ "tree", "--pid", "11" }), { app(10), app(11) }).pid, 11u);
}

TEST(ArgsTest, SelectAppFailsWithCandidates) {
    try {
        selectApp(parse({ "tree" }), { app(10), app(11) });
        FAIL();
    } catch (const RpcError& e) {
        EXPECT_EQ(e.data()["candidates"].asArray().size(), 2u);
        EXPECT_FALSE(e.data()["hint"].asString().empty());
    }
    EXPECT_THROW(selectApp(parse({ "tree" }), {}), RpcError);
    EXPECT_THROW(selectApp(parse({ "tree", "--pid", "99" }), { app(10) }), RpcError);
}
