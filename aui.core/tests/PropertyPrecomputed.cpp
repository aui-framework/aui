// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Common/AProperty.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <gmock/gmock.h>

class PropertyPrecomputedTest : public testing::Test {
public:
    /*
        template<typename... Args>
        static auto& connections(ASignal<Args...>& signal) {
            return signal.mOutgoingConnections;
        }

        static auto& connections(AObject& object) {
            return object.mIngoingConnections;
        }*/
};

namespace {
// AUI_DOCS_OUTPUT: doxygen/intermediate/property_precomputed.h
// @class APropertyPrecomputed
// # Declaration
// Declare a property with custom expression determining it's value as follows:
// AUI_DOCS_CODE_BEGIN
struct User {
    AProperty<AString> name;
    AProperty<AString> surname;
    APropertyPrecomputed<AString> fullName = [&] { return "{} {}"_format(name, surname); };
};
// AUI_DOCS_CODE_END

class LogObserver : public AObject {
public:
    LogObserver() {
        ON_CALL(*this, log(testing::_)).WillByDefault([](const AString& msg) {});
    }
    MOCK_METHOD(void, log, (const AString& msg), ());
};
}

TEST_F(PropertyPrecomputedTest, APropertyPrecomputed) {
    // Let's make little observer object for demonstration:
    {
        // AUI_DOCS_CODE_BEGIN
        class LogObserver : public AObject {
        public:
            void log(const AString& msg) {
                ALogger::info("LogObserver") << "Received value: " << msg;
            }
        };
        // AUI_DOCS_CODE_END
    }
    testing::InSequence s;

    // Usage:
    // AUI_DOCS_CODE_BEGIN
    auto u = aui::ptr::manage(new User {
        .name = "Emma",
        .surname = "Watson",
    });

    auto observer = _new<LogObserver>();
    EXPECT_CALL(*observer, log(AString("Emma Watson"))).Times(1);
    AObject::connect(u->fullName, slot(observer)::log);
    // AUI_DOCS_CODE_END
    EXPECT_EQ(u->fullName, "Emma Watson");
    //
    // The example above prints "Emma Watson". If we try to update one of dependencies of `APropertyPrecomputed` (i.e.,
    // `name` or `surname`), `APropertyPrecomputed` responds immediately:

    // AUI_DOCS_CODE_BEGIN
    EXPECT_CALL(*observer, log(AString("Emma Stone"))).Times(1);
    u->surname = "Stone";
    // AUI_DOCS_CODE_END
    //
    // The example above prints "Emma Stone".
    EXPECT_EQ(u->fullName, "Emma Stone");
}

TEST_F(PropertyPrecomputedTest, Valid_Expressions) { // HEADER_H1
    // Any C++ callable evaluating to `T` can be used as an expression for `APropertyPrecomputed<T>`. However, to
    // formulate correct expression, some rules must be satisfied.
    //
    // Dependency tracking only works on other properties. It is the developer's responsibility to ensure all values
    // referenced in the expression are properties, or, at least, non-property values that wouldn't change or whose
    // changes are not interesting. You definitely can use branching inside the expression, but you must be confident
    // about what are you doing. Generally speaking, use as trivial expressions as possible.
    //
    // AUI_DOCS_CODE_BEGIN
    struct User {
        AProperty<AString> name;
        AProperty<AString> surname;
        APropertyPrecomputed<AString> fullName = [&]() -> AString {
            if (name->empty()) {
                return "-";
            }
            if (surname->empty()) {
                return "-";
            }
            return "{} {}"_format(name, surname);
        };
    };
    // AUI_DOCS_CODE_END
    //
    // In this expression, we have a fast path return if `name` is empty.
    // AUI_DOCS_CODE_BEGIN
    User u = {
        .name = "Emma",
        .surname = "Watson",
    };
    // trivial: we've accessed all referenced properties
    EXPECT_EQ(u.fullName, "Emma Watson");
    // AUI_DOCS_CODE_END

    EXPECT_EQ(u.name.changed.mOutgoingConnections.size(), 1);
    EXPECT_EQ(u.surname.changed.mOutgoingConnections.size(), 1);

    // As soon as we set `name` to `""`, we don't access `surname`. If we try to trigger the fast path return:
    // AUI_DOCS_CODE_BEGIN
    u.name = "";
    // AUI_DOCS_CODE_END
    EXPECT_EQ(u.fullName, "-");
    EXPECT_EQ(u.name.changed.mOutgoingConnections.size(), 1);
    EXPECT_EQ(u.surname.changed.mOutgoingConnections.size(), 0);
    // `surname` can't trigger re-evaluation anyhow. Re-evaluation can be triggered by `name` only. So, at the moment,
    // we are interested in `name` changes only.
    //
    // `APropertyPrecomputed` might evaluate its expression several times during its lifetime. The developer must make
    // sure that all objects referenced in the expression live longer than `APropertyPrecomputed`.
    //
    // The expression should not read from the property it's a binding for. Otherwise, there's an infinite evaluation
    // loop.
}

