#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "remote/Skill.h"
#include "TestUtil.h"

using namespace aui_remote;

TEST(SkillTest, RenderReplacesExePlaceholder) {
    EXPECT_EQ(skill::render("run {{EXE}} cli list; {{EXE}} cli tree", "/opt/aui-remote"),
              "run /opt/aui-remote cli list; /opt/aui-remote cli tree");
}

TEST(SkillTest, InstalledStateComesFromTheFile) {
    test::TempDir dir;
    const auto file = dir / "skills" / "aui-remote" / "SKILL.md";
    EXPECT_FALSE(skill::isInstalled(file));
    skill::install("exe: {{EXE}}", "/bin/x", file);
    EXPECT_TRUE(skill::isInstalled(file));

    std::ifstream in(file.toStdString());
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "exe: /bin/x");

    // somebody deletes the file by hand: the state follows
    file.removeFile();
    EXPECT_FALSE(skill::isInstalled(file));
}

TEST(SkillTest, UninstallRemovesFileAndEmptyFolder) {
    test::TempDir dir;
    const auto file = dir / "skills" / "aui-remote" / "SKILL.md";
    skill::install("x", "/bin/x", file);
    skill::uninstall(file);
    EXPECT_FALSE(skill::isInstalled(file));
    EXPECT_FALSE(file.parent().isDirectoryExists());
    skill::uninstall(file);   // not an error
}

TEST(SkillTest, UninstallKeepsFolderWithOtherFiles) {
    test::TempDir dir;
    const auto file = dir / "skills" / "aui-remote" / "SKILL.md";
    skill::install("x", "/bin/x", file);
    std::ofstream((file.parent() / "extra.txt").toStdString()) << "mine";
    skill::uninstall(file);
    EXPECT_TRUE((file.parent() / "extra.txt").isRegularFileExists());
}

TEST(SkillTest, DefaultFileHonoursClaudeConfigDir) {
    ::setenv("CLAUDE_CONFIG_DIR", "/tmp/claude-cfg", 1);
    EXPECT_EQ(skill::defaultFile(), APath("/tmp/claude-cfg/skills/aui-remote/SKILL.md"));
    ::unsetenv("CLAUDE_CONFIG_DIR");
    EXPECT_TRUE(skill::defaultFile().endsWith(".claude/skills/aui-remote/SKILL.md"));
}
