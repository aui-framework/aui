#include <gtest/gtest.h>

#include "cli/TreeText.h"

using namespace aui_remote;

TEST(TreeTextTest, OneLinePerViewWithIndent) {
    auto json = AJson::fromString(R"({
      "ref":"v3:2","type":"AWindow","geometry":{"x":0,"y":0,"w":900,"h":450,"absX":0,"absY":0},
      "visible":true,"enabled":true,"focused":true,"assNames":["noise"],
      "children":[
        {"ref":"v4:9","type":"ALabel","text":"Value: 0","geometry":{"x":0,"y":0,"w":695,"h":22,"absX":102,"absY":170},
         "visible":true,"enabled":true,"focused":false},
        {"ref":"v4:10","type":"AButton","geometry":{"x":0,"y":34,"w":695,"h":33,"absX":102,"absY":204},
         "visible":true,"enabled":false,"focused":false,"role":"button","actions":["clickable"],
         "children":[{"ref":"v4:11","type":"ALabel","text":"Click me","visible":false}]}
      ]})");
    EXPECT_EQ(renderTreeText(json),
              "AWindow [v3:2] 900x450 @0,0 focused\n"
              "  ALabel \"Value: 0\" [v4:9] 695x22 @102,170\n"
              "  AButton [v4:10] 695x33 @102,204 disabled clickable\n"
              "    ALabel \"Click me\" [v4:11] hidden\n");
}

TEST(TreeTextTest, LongTextIsShortenedAndQuoted) {
    auto json = AJson { { "type", "ALabel" }, { "text", AString(200, 'x') + "\n\"" } };
    auto text = renderTreeText(json);
    EXPECT_LT(text.length(), 120u);
    EXPECT_EQ(text.find('\n'), text.length() - 1);   // the newline of the text is escaped
}
