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

//
// Created by Alex2772 on 12/18/2021.
//

#pragma once


#include <gtest/gtest.h>

#include <AUI/Platform/AWindow.h>
#include <AUI/Software/SoftwareRenderer.h>


namespace testing {
    class API_AUI_UITESTS UITest : public testing::Test {
    public:
        ~UITest() override = default;

    protected:
        void SetUp() override;
        void TearDown() override;

        void saveScreenshot(const AString& name);

    public:

    };
}