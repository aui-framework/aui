// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
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
#include "AUI/ARuntimeBorrowChecker.h"

class RuntimeBorrowChecker : public testing::Test {
public:
};

// AUI_DOCS_OUTPUT: doxygen/intermediate/exclusive_access_lockable.h
// @class ARuntimeBorrowChecker

TEST_F(RuntimeBorrowChecker, Principle) {   // HEADER_H1
    // AUI_DOCS_CODE_BEGIN
    struct SharedResource {
        AString data;
    };
    ARuntimeBorrowChecker<SharedResource> sharedResource;

    if (auto read1 = sharedResource.readScope()) {   // ok
        /* // HIDE
        read1->data = "Hello"; // can't
        */ // HIDE
        [[maybe_unused]] auto length1 = read1->data.length();   // ok

        if (auto read2 = sharedResource.readScope()) {         // ok, second read
            [[maybe_unused]] auto length2 = read2->data.length();   // ok

            if (auto write = sharedResource.writeScope()) { // bad, there are reads in progress
                // this branch will not be executed, since we've failed to lock
                GTEST_FAIL() << "should not reach here"; // HIDE
            } else {
                // instead, it will go here.
                // it is up to you how you want to handle such case.
                // throw AException("cannot write shared resource");
            }
        } else {
            throw AException("cannot read shared resource");
        }
    } else {
        throw AException("cannot read shared resource");
    }
    ok:

    if (auto write = sharedResource.writeScope()) { // ok
        write->data = "hell";
    } else {
        throw AException("can't write to shared resource");
    }
    // AUI_DOCS_CODE_END
    if (auto write = sharedResource.writeScope()) { // ok
        write->data = "hell";
    } else {
        throw AException("can't write to shared resource");
    }
    //
    // Always check if scope acquisition succeeded and handle just in case.
    //
    // Use the appropriate scope type (read/write) for intended operations. Prefer read scopes when write access isn't
    // necessarily needed.
}

/*
TEST_F(RuntimeBorrowChecker, Usage) {   // HEADER_H1

    // In this example, we are ensuring the integrity of for each loop iteration:
    // AUI_DOCS_CODE_BEGIN
    struct SharedResource {
        AVector<AString> users;
    };
    ARuntimeBorrowChecker<SharedResource> sharedResource;

    auto goodFunction = [] {

    };

    auto handleUsers = [&](const std::function<void(const AString&)>& handler) {
        if (auto sharedResourceRead = sharedResource.readScope()) {
            for (const auto& user : sharedResourceRead->users) {
                handler(user);   // ok, but if handler happens to modify users, it
                                 // would break our foreach loop
            }
        } else {
            throw AException("cannot read shared resource");
        }
    };

    handleUsers(goodFunction);

    // AUI_DOCS_CODE_END
}
*/