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

class ExclusiveAccessLockable : public testing::Test {
public:
};

// AUI_DOCS_OUTPUT: doxygen/intermediate/exclusive_access_lockable.h
// @class AExclusiveAccessLockableTest


TEST_F(ExclusiveAccessLockable, Usage) { // HEADER_H1
    // AUI_DOCS_CODE_BEGIN
     struct SharedResource {
       AVector<AString> users;
     };
     AExclusiveAccess<SharedResource, AMutex> sharedResource;
    // AUI_DOCS_CODE_END
}