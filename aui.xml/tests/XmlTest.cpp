/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Xml/AXml.h>
#include <AUI/Xml/AXmlParseError.h>
#include <AUI/Xml/IXmlDocumentVisitor.h>
#include <AUI/Xml/IXmlEntityVisitor.h>

namespace {
/// Records everything the parser reports as a flat list of events.
struct Recorder: IXmlEntityVisitor {
    AString name;
    AVector<AString>& log;
    Recorder(AString name, AVector<AString>& log): name(std::move(name)), log(log) {}

    void visitAttribute(const AString& attrName, AString value) override {
        log << name + "@" + attrName + "=" + value;
    }
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        log << name + "/" + entityName;
        return _new<Recorder>(std::move(entityName), log);
    }
    void visitTextEntity(const AString& entity) override {
        log << name + ":" + entity;
    }
};

struct RecorderDocument: IXmlDocumentVisitor {
    AVector<AString> log;
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        log << "/" + entityName;
        return _new<Recorder>(std::move(entityName), log);
    }
};

AVector<AString> parse(const AString& xml) {
    auto visitor = _new<RecorderDocument>();
    AXml::read(_new<AStringStream>(xml), visitor);
    return visitor->log;
}
}   // namespace

TEST(Xml, TextRightAfterOpeningTag) {
    // the first character of the text must not be lost
    EXPECT_EQ(parse("<a>hello</a>"), (AVector<AString>{ "/a", "a:hello" }));
}

TEST(Xml, TextAfterChild) {
    EXPECT_EQ(parse("<a><b/>tail</a>"), (AVector<AString>{ "/a", "a/b", "a:tail" }));
}

TEST(Xml, Attributes) {
    EXPECT_EQ(parse(R"(<a x="1" y='2'/>)"), (AVector<AString>{ "/a", "a@x=1", "a@y=2" }));
}

namespace {
struct Skipper: IXmlEntityVisitor {
    AVector<AString> log;
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        if (entityName == "skip") {
            return nullptr;   // ignore this element along with everything inside
        }
        log << "/" + entityName;
        return _new<Skipper>();
    }
    void visitAttribute(const AString& attrName, AString value) override {
        log << "@" + attrName;
    }
    void visitTextEntity(const AString& entity) override {
        if (!entity.empty()) {   // the parser reports empty text between elements; irrelevant here
            log << ":" + entity;
        }
    }
};
struct SkipperDocument: IXmlDocumentVisitor {
    _<Skipper> root = _new<Skipper>();
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        return root;
    }
};
}   // namespace

TEST(Xml, NullVisitorSkipsTheWholeElement) {
    auto visitor = _new<SkipperDocument>();
    AXml::read(_new<AStringStream>(AString(R"(<a><skip x="1"><b>inner</b></skip><c/></a>)")), visitor);
    // neither attributes, children, nor the text of <skip> leak into <a>; <c> is still visited
    EXPECT_EQ(visitor->root->log, (AVector<AString>{ "/c" }));
}

TEST(Xml, ErrorPositionHasRowAndColumn) {
    try {
        parse("<a>\n<b x=1/></a>");
        FAIL() << "expected AXmlParseError";
    } catch (const AXmlParseError& e) {
        // "... at <row>:<column>"; the column of an error in the second line can't be the row number
        AString msg = e.what();
        EXPECT_TRUE(msg.contains(" at 2:")) << msg;
        EXPECT_FALSE(msg.contains(" at 2:2")) << msg;
    }
}
