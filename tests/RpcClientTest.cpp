#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include <AUI/Platform/AProcess.h>
#include <sstream>

#include "cli/Cli.h"
#include "FakeAgent.h"
#include "remote/KeyStore.h"
#include "remote/RpcClient.h"
#include "TestUtil.h"

using namespace aui_remote;
using namespace aui_remote::test;

namespace {

/// Runs the CLI and returns the exit code and the last stdout line parsed as JSON.
struct CliResult {
    int exitCode;
    AJson json;
    std::string stdoutText;
};

CliResult runCli(std::initializer_list<const char*> args, const cli::Directories& directories) {
    AStringVector v;
    for (auto a : args) {
        v << a;
    }
    std::stringstream captured;
    auto* old = std::cout.rdbuf(captured.rdbuf());
    auto* oldErr = std::cerr.rdbuf(nullptr);
    int code = cli::run(v, directories);
    std::cout.rdbuf(old);
    std::cerr.rdbuf(oldErr);
    CliResult result { code, {}, captured.str() };
    std::string lastLine, line;
    std::stringstream lines(result.stdoutText);
    while (std::getline(lines, line)) {
        if (!line.empty()) {
            lastLine = line;
        }
    }
    result.json = AJson::fromString(AString::fromUtf8(lastLine));
    return result;
}

void writeDiscovery(const APath& dir, std::uint32_t pid, std::uint16_t port, const AString& cert = {}) {
    dir.makeDirs();
    std::ofstream(((dir / (AString::number(pid) + ".json"))).toStdString())
        << R"({"protocol":1,"pid":)" << pid << R"(,"port":)" << port
        << R"(,"scheme":"wss","certSha256":")" << cert.toStdString()
        << R"(","exe":"/bin/fake","argv":["/bin/fake"],"cwd":"/","startedAt":"t0"})";
}

}   // namespace

TEST(RpcClientTest, CallsWithKey) {
    FakeAgent agent("goodkey");
    RpcClient client(agent.port(), agent.certSha256(), "goodkey");
    auto result = client.call("window.list");
    EXPECT_EQ(result["windows"][0]["title"].asString(), "Fake");
}

TEST(RpcClientTest, SeveralRequestsShareTheConnection) {
    FakeAgent agent("goodkey");
    RpcClient client(agent.port(), agent.certSha256(), "goodkey");
    auto results = client.callSequence({ { "window.list", {} }, { "view.tree", AJson { { "depth", 4 } } } });
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[1]["echoDepth"].asInt(), 4);
}

TEST(RpcClientTest, ErrorsCarryCodeAndHint) {
    FakeAgent agent("goodkey");
    RpcClient client(agent.port(), agent.certSha256(), "goodkey");
    try {
        client.call("fail");
        FAIL();
    } catch (const RpcError& e) {
        EXPECT_EQ(e.code(), -32002);
        EXPECT_EQ(e.data()["hint"].asString(), "find again");
    }
}

TEST(RpcClientTest, UnauthorizedConnectionCanOnlyPair) {
    FakeAgent agent;
    RpcClient client(agent.port(), agent.certSha256(), "");
    try {
        client.call("window.list");
        FAIL();
    } catch (const RpcError& e) {
        EXPECT_EQ(e.code(), rpc_code::UNAUTHORIZED);
    }
    EXPECT_TRUE(client.call("pair.begin")["pending"].asBool());
}

TEST(RpcClientTest, WrongCertificateIsRejected) {
    FakeAgent agent("goodkey");
    FakeAgent other;
    try {
        RpcClient(agent.port(), other.certSha256(), "goodkey").call("window.list");
        FAIL();
    } catch (const RpcError& e) {
        EXPECT_NE(e.getMessage().find("secure connection failed"), AString::NPOS);
    }
}

TEST(RpcClientTest, RefusesAgentWithoutPinnedWss) {
    AppInfo app;
    app.pid = 1;
    app.port = 1;
    EXPECT_THROW(RpcClient::forApp(app, "k"), RpcError);
    app.scheme = "ws";
    app.certSha256 = AString(64, 'a');
    EXPECT_THROW(RpcClient::forApp(app, "k"), RpcError);
    app.scheme = "wss";
    EXPECT_NO_THROW(RpcClient::forApp(app, "k"));
    app.certSha256 = "abcd";
    EXPECT_THROW(RpcClient::forApp(app, "k"), RpcError);
}

TEST(RpcClientTest, ConnectionToNothingFails) {
    // a port nobody listens to: the fake agent is gone
    std::uint16_t port;
    AString cert;
    {
        FakeAgent agent;
        port = agent.port();
        cert = agent.certSha256();
    }
    EXPECT_THROW(RpcClient(port, cert, "k").call("window.list", {}, std::chrono::seconds(5)), RpcError);
}

TEST(CliTest, ListShowsPairedFlagAndPrunesDeadApps) {
    TempDir dir;
    cli::Directories d { dir / "apps", dir / "keys" };
    writeDiscovery(d.discovery, AProcess::self()->getPid(), 1234);
    writeDiscovery(d.discovery, 0x3ffffff0, 4321);   // dead
    auto r = runCli({ "list" }, d);
    ASSERT_EQ(r.exitCode, 0);
    ASSERT_EQ(r.json["apps"].asArray().size(), 1u);
    EXPECT_FALSE(r.json["apps"][0]["paired"].asBool());
    EXPECT_FALSE((d.discovery / "1073741808.json").isRegularFileExists());

    auto app = discovery::list(d.discovery).first();
    KeyStore(d.keys).store(app, "k");
    EXPECT_TRUE(runCli({ "list" }, d).json["apps"][0]["paired"].asBool());
}

TEST(CliTest, PairingFlow) {
    TempDir dir;
    FakeAgent agent;
    cli::Directories d { dir / "apps", dir / "keys" };
    const auto pid = AProcess::self()->getPid();
    writeDiscovery(d.discovery, pid, agent.port(), agent.certSha256());
    const AString pidText = AString::number(pid);

    // not paired yet: a command is refused locally
    auto refused = runCli({ "windows" }, d);
    EXPECT_EQ(refused.exitCode, 1);
    EXPECT_EQ(refused.json["error"]["code"].asInt(), rpc_code::UNAUTHORIZED);

    auto begin = runCli({ "pair", pidText.toStdString().c_str() }, d);
    ASSERT_EQ(begin.exitCode, 0) << begin.stdoutText;
    EXPECT_TRUE(begin.json["pending"].asBool());
    EXPECT_EQ(agent.pairBeginCalls(), 1);

    auto wrong = runCli({ "pair", pidText.toStdString().c_str(), "--code", "000000" }, d);
    EXPECT_EQ(wrong.exitCode, 1);
    EXPECT_EQ(wrong.json["error"]["data"]["attemptsLeft"].asInt(), 2);
    EXPECT_TRUE(KeyStore(d.keys).directory().isDirectoryExists() == false ||
                KeyStore(d.keys).directory().listDir().empty());

    auto ok = runCli({ "pair", pidText.toStdString().c_str(), "--code", FakeAgent::CODE }, d);
    ASSERT_EQ(ok.exitCode, 0) << ok.stdoutText;
    EXPECT_TRUE(ok.json["paired"].asBool());

    auto app = discovery::list(d.discovery).first();
    auto key = KeyStore(d.keys).load(app);
    ASSERT_TRUE(key);
    EXPECT_TRUE(agent.hasKey(*key));
    EXPECT_TRUE(ok.json["remembered"].asBool());
    EXPECT_TRUE(KeyStore(d.keys).pathFor(app).isRegularFileExists());   // the one-time key, not a session one

    // already paired: no code, no new key
    auto again = runCli({ "pair", pidText.toStdString().c_str() }, d);
    ASSERT_EQ(again.exitCode, 0) << again.stdoutText;
    EXPECT_TRUE(again.json["alreadyPaired"].asBool());
    EXPECT_EQ(agent.pairBeginCalls(), 1);   // still the one from the first pair

    // the stored key is used by the later invocations
    auto windows = runCli({ "windows" }, d);
    ASSERT_EQ(windows.exitCode, 0) << windows.stdoutText;
    EXPECT_EQ(windows.json["windows"][0]["title"].asString(), "Fake");
    auto tree = runCli({ "tree", "--depth", "2" }, d);
    EXPECT_EQ(tree.json["echoDepth"].asInt(), 2);
}

TEST(CliTest, ForgottenClientIsAskedToPairAgain) {
    TempDir dir;
    FakeAgent agent;
    cli::Directories d { dir / "apps", dir / "keys" };
    const auto pid = AProcess::self()->getPid();
    writeDiscovery(d.discovery, pid, agent.port(), agent.certSha256());
    auto app = discovery::list(d.discovery).first();
    KeyStore(d.keys).store(app, "key-the-application-has-never-seen");   // i.e. it forgot after a restart

    auto r = runCli({ "windows" }, d);
    EXPECT_EQ(r.exitCode, 1);
    EXPECT_EQ(r.json["error"]["code"].asInt(), rpc_code::UNAUTHORIZED);
    EXPECT_TRUE(r.json["error"]["data"]["hint"].asString().contains("pair"));
    EXPECT_FALSE(KeyStore(d.keys).load(app));   // the stale key is dropped

    // pair then asks for the code instead of claiming it is paired
    KeyStore(d.keys).store(app, "key-the-application-has-never-seen");
    auto begin = runCli({ "pair", AString::number(pid).toStdString().c_str() }, d);
    ASSERT_EQ(begin.exitCode, 0) << begin.stdoutText;
    EXPECT_TRUE(begin.json["pending"].asBool());
}

TEST(CliTest, ErrorsAreJsonWithNonZeroExit) {
    TempDir dir;
    cli::Directories d { dir / "apps", dir / "keys" };
    auto none = runCli({ "tree" }, d);
    EXPECT_EQ(none.exitCode, 1);
    EXPECT_TRUE(none.json["error"]["data"].containsOpt("hint"));

    auto bad = runCli({ "click" }, d);
    EXPECT_EQ(bad.exitCode, 2);
    EXPECT_TRUE(bad.json.containsOpt("error"));

    auto unknown = runCli({ "frobnicate" }, d);
    EXPECT_EQ(unknown.exitCode, 2);
}

TEST(CliTest, TryRunIgnoresGuiLaunch) {
    EXPECT_FALSE(cli::tryRun({ "aui-remote" }).hasValue());
    EXPECT_FALSE(cli::tryRun({ "aui-remote", "--something" }).hasValue());
}
