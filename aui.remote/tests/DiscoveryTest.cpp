#include <gtest/gtest.h>

#include <fstream>

#include <AUI/Platform/AProcess.h>

#include "network/Discovery.h"
#include "TestUtil.h"

using namespace aui_remote;

namespace {
void write(const APath& file, const std::string& content) {
    std::ofstream(file.toStdString()) << content;
}

std::string json(int pid, int port) {
    return R"({"protocol":1,"pid":)" + std::to_string(pid) + R"(,"port":)" + std::to_string(port) +
           R"(,"scheme":"wss","certSha256":")" + std::string(64, 'A') + R"(","exe":"/usr/bin/app","argv":["/usr/bin/app","--x"],"cwd":"/home","startedAt":"2026-10-25T10:00:00Z"})";
}
}   // namespace

TEST(DiscoveryTest, ParsesDiscoveryFile) {
    test::TempDir dir;
    write(dir / "10.json", json(10, 4242));
    auto apps = discovery::list(dir.path(), [](std::uint32_t) { return true; });
    ASSERT_EQ(apps.size(), 1u);
    EXPECT_EQ(apps[0].pid, 10u);
    EXPECT_EQ(apps[0].port, 4242);
    EXPECT_EQ(apps[0].exe, "/usr/bin/app");
    EXPECT_EQ(apps[0].argv, (AStringVector { "/usr/bin/app", "--x" }));
    EXPECT_EQ(apps[0].cwd, "/home");
    EXPECT_EQ(apps[0].startedAt, "2026-10-25T10:00:00Z");
    EXPECT_EQ(apps[0].displayName(), "app (10)");
    EXPECT_EQ(apps[0].scheme, "wss");
    EXPECT_EQ(apps[0].certSha256, AString(64, 'a'));   // normalized to lowercase
    EXPECT_TRUE(apps[0].isSupported());
}

TEST(DiscoveryTest, StartedAtMayBeANumber) {
    auto info = AppInfo::fromJson(AJson::fromString(R"({"pid":5,"port":6,"startedAt":1761390000})"));
    EXPECT_EQ(info.startedAt, "1761390000");
}

TEST(DiscoveryTest, DeadProcessesArePruned) {
    test::TempDir dir;
    write(dir / "10.json", json(10, 1));
    write(dir / "11.json", json(11, 2));
    write(dir / "12.json", json(12, 3));
    auto apps = discovery::list(dir.path(), [](std::uint32_t pid) { return pid != 11; });
    ASSERT_EQ(apps.size(), 2u);
    EXPECT_EQ(apps[0].pid, 10u);
    EXPECT_EQ(apps[1].pid, 12u);
    EXPECT_TRUE((dir / "10.json").isRegularFileExists());
    EXPECT_FALSE((dir / "11.json").isRegularFileExists());
    EXPECT_TRUE((dir / "12.json").isRegularFileExists());
}

TEST(DiscoveryTest, GarbageIsSkippedButKept) {
    test::TempDir dir;
    write(dir / "1.json", "not json");
    write(dir / "2.json", R"({"pid":2})");
    write(dir / "notes.txt", "hello");
    EXPECT_TRUE(discovery::list(dir.path(), [](std::uint32_t) { return false; }).empty());
    EXPECT_TRUE((dir / "1.json").isRegularFileExists());
    EXPECT_TRUE((dir / "2.json").isRegularFileExists());
    EXPECT_TRUE((dir / "notes.txt").isRegularFileExists());
}

TEST(DiscoveryTest, MissingDirectoryIsEmpty) {
    test::TempDir dir;
    EXPECT_TRUE(discovery::list(dir / "nope").empty());
}

TEST(DiscoveryTest, FindByPid) {
    test::TempDir dir;
    write(dir / "10.json", json(10, 4242));
    EXPECT_EQ(discovery::find(10, dir.path())->port, 4242);
    EXPECT_FALSE(discovery::find(11, dir.path()));
}

TEST(DiscoveryTest, RealPidCheck) {
    EXPECT_TRUE(discovery::isProcessAlive(AProcess::self()->getPid()));
    EXPECT_FALSE(discovery::isProcessAlive(0x3ffffff0));
}
