#include <gtest/gtest.h>

#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <AUI/Platform/AProcess.h>
#include <AUI/Remote/ARemoteDebug.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>

#include "cli/Cli.h"
#include "network/KeyStore.h"
#include "TestUtil.h"

using namespace aui_remote;
using namespace declarative;
using namespace std::chrono_literals;

namespace {
// the agent is a singleton shared with the tests of aui.dev_extras, so it is started with the same options
constexpr auto KEY = "test-key-0123456789abcdef";

_<ARemoteDebug> agent() {
    ARemoteDebug::Options options;
    options.writeDiscoveryFile = false;
    options.pairing = ARemoteDebug::PairingPolicy::SESSION;
    options.key = KEY;
    return ARemoteDebug::start(std::move(options));
}
}   // namespace

/**
 * The CLI of aui-remote against the real remote agent of aui::dev_extras, so the client and the protocol can't drift
 * apart.
 */
class EndToEndTest : public testing::UITest {
protected:
    test::TempDir mDir;
    cli::Directories mDirectories { mDir / "apps", mDir / "keys" };

    void SetUp() override {
        UITest::SetUp();
        auto a = agent();
        mDirectories.discovery.makeDirs();
        const auto pid = AProcess::self()->getPid();
        std::ofstream((mDirectories.discovery / (AString::number(pid) + ".json")).toStdString())
            << R"({"protocol":1,"pid":)" << pid << R"(,"port":)" << a->port() << R"(,"scheme":"wss","certSha256":")"
            << a->certificateFingerprint().toStdString()
            << R"(","exe":"/bin/e2e","argv":["/bin/e2e"],"cwd":"/","startedAt":"t0"})";
        KeyStore(mDirectories.keys).storeSession(discovery::list(mDirectories.discovery).first(), KEY);
    }

    struct Result {
        int exitCode = -1;
        AJson json;
        std::string text;
    };

    /// Runs the CLI on another thread while this (UI) thread serves the agent.
    Result cli(std::initializer_list<const char*> args) {
        AStringVector v;
        for (auto a : args) {
            v << a;
        }
        Result result;
        std::stringstream captured;
        std::atomic_bool done = false;
        auto* oldOut = std::cout.rdbuf(captured.rdbuf());
        auto* oldErr = std::cerr.rdbuf(nullptr);
        std::thread thread([&] {
            result.exitCode = cli::run(v, mDirectories);
            done = true;
        });
        for (int i = 0; i < 5000 && !done; ++i) {
            uitest::frame();
            std::this_thread::sleep_for(2ms);
        }
        thread.join();
        std::cout.rdbuf(oldOut);
        std::cerr.rdbuf(oldErr);
        result.text = captured.str();
        std::string last, line;
        std::stringstream lines(result.text);
        while (std::getline(lines, line)) {
            if (!line.empty()) {
                last = line;
            }
        }
        if (!last.empty() && (last[0] == '{' || last[0] == '[')) {
            result.json = AJson::fromString(AString::fromUtf8(last));
        }
        return result;
    }

    _<AWindow> showWindow(_<AView> contents) {
        auto window = _new<AWindow>("End to end", 400_px, 300_px);
        window->setContents(Centered { std::move(contents) });
        window->show();
        uitest::frame();
        return window;
    }
};

TEST_F(EndToEndTest, WindowsTreeAndClick) {
    int clicks = 0;
    auto button = _new<AButton>("Press me");
    AObject::connect(button->clicked, button, [&] { ++clicks; });
    auto window = showWindow(Vertical { Label { "Hello" }, button });

    auto windows = cli({ "windows" });
    ASSERT_EQ(windows.exitCode, 0) << windows.text;
    bool found = false;
    for (const auto& w : windows.json.asArray()) {
        found |= w["title"].asString() == "End to end";
    }
    EXPECT_TRUE(found) << windows.text;

    auto tree = cli({ "tree" });
    ASSERT_EQ(tree.exitCode, 0) << tree.text;
    EXPECT_NE(tree.text.find("Press me"), std::string::npos) << tree.text;

    auto find = cli({ "find", "--type", "Button", "--text", "Press me" });
    ASSERT_EQ(find.exitCode, 0) << find.text;
    ASSERT_EQ(find.json.asArray().size(), 1u) << find.text;
    const auto ref = find.json[0]["ref"].asString().toStdString();

    auto click = cli({ "click", ref.c_str() });
    ASSERT_EQ(click.exitCode, 0) << click.text;
    EXPECT_EQ(clicks, 1);
}

TEST_F(EndToEndTest, ScreenshotIsSaved) {
    auto window = showWindow(Label { "picture" });
    auto shot = cli({ "screenshot", "--annotate" });
    ASSERT_EQ(shot.exitCode, 0) << shot.text;
    EXPECT_TRUE(APath(shot.json["path"].asString()).isRegularFileExists()) << shot.text;
    EXPECT_TRUE(shot.json.contains("marks")) << shot.text;
}

TEST_F(EndToEndTest, ErrorsOfTheAgentReachTheCli) {
    auto window = showWindow(Label { "x" });
    auto r = cli({ "props", "v0:99999" });
    EXPECT_NE(r.exitCode, 0);
    EXPECT_TRUE(r.json["error"]["data"].contains("hint")) << r.text;
}
