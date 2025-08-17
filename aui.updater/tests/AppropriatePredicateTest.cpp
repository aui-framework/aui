#include <gmock/gmock.h>
#include <AUI/Updater/AppropriatePortablePackagePredicate.h>
#include <range/v3/algorithm/find_if.hpp>
#include <AUI/Common/AStringVector.h>

// AUI_DOCS_OUTPUT: doxygen/intermediate/appropriate_predicate.h
// @struct aui::updater::AppropriatePortablePackagePredicate

TEST(AppropriatePredicateTest, Custom_qualifier) {   // HEADER_H2
    // AUI_DOCS_CODE_BEGIN
    aui::updater::AppropriatePortablePackagePredicate p { .qualifier = "windows-x86_64" };
    EXPECT_TRUE(p("app-windows-x86_64.zip"));
    EXPECT_FALSE(p("app-windows-x86.zip"));
    // AUI_DOCS_CODE_END
}

TEST(AppropriatePredicateTest, Typical_usage_with_default_qualifier) { // HEADER_H2
    // It's convenient to use it with `find_if`:
    // AUI_DOCS_CODE_BEGIN
    struct Response {
        AString fileName;
        AString downloadUrl;
    } foundFiles[] = {
        { "app-some_os-x86_64-portable.zip", "https://..." },
        { "app-some_os-arm64-portable.zip", "https://..." },
    };
    auto foundFilesIt =
        ranges::find_if(foundFiles, aui::updater::AppropriatePortablePackagePredicate {}, &Response::fileName);
    // AUI_DOCS_CODE_END
    EXPECT_EQ(foundFilesIt, ranges::end(foundFiles));
}
