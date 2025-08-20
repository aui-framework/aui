#include <gmock/gmock.h>
#include <AUI/Updater/Semver.h>

TEST(VersionTest, Parsing_version) { // HEADER_H2
    // Up to 3 numbers split with dots are supported:
    //AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(aui::updater::Semver::fromString("1.2.3"), (aui::updater::Semver{ 1, 2, 3 }));
    EXPECT_EQ(aui::updater::Semver::fromString("1.2"), (aui::updater::Semver{ 1, 2, 0 }));
    EXPECT_EQ(aui::updater::Semver::fromString("1"), (aui::updater::Semver{ 1, 0, 0 }));
    //AUI_DOCS_CODE_END

    // `v` prefix is acceptable:
    //AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(aui::updater::Semver::fromString("v1.2.3"), (aui::updater::Semver{ 1, 2, 3 }));
    EXPECT_EQ(aui::updater::Semver::fromString("v0.0.0"), (aui::updater::Semver{ 0, 0, 0 }));
    //AUI_DOCS_CODE_END

    // "Channel names" and other suffixes are ignored:
    //AUI_DOCS_CODE_BEGIN
    EXPECT_EQ(aui::updater::Semver::fromString("1.2.3-beta"), (aui::updater::Semver{ 1, 2, 3 }));
    //AUI_DOCS_CODE_END
}
