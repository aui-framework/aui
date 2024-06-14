/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/**
 * @defgroup uitests aui::uitests
 * @brief ui tests library.
 */

#include <AUI/View/AView.h>

#include <AUI/Test/UI/UITestUtil.h>

#include <gtest/gtest.h>
#include <AUI/Test/UI/UIMatcher.h>
#include <AUI/Test/UI/By.h>
#include <AUI/Test/UI/UITestCase.h>

#include <AUI/Test/UI/Action/Click.h>
#include <AUI/Test/UI/Action/Gesture.h>
#include <AUI/Test/UI/Action/DoubleClick.h>
#include <AUI/Test/UI/Action/Type.h>
#include <AUI/Test/UI/Action/MouseMove.h>
#include <AUI/Test/UI/Action/Scroll.h>
#include <AUI/Test/UI/Action/KeyDownAndUp.h>

#include <AUI/Test/UI/Assertion/Align.h>
#include <AUI/Test/UI/Assertion/Empty.h>
#include <AUI/Test/UI/Assertion/Gone.h>
#include <AUI/Test/UI/Assertion/Visible.h>
#include <AUI/Test/UI/Assertion/NotVisible.h>
#include <AUI/Test/UI/Assertion/Text.h>
#include <AUI/Test/UI/Assertion/Size.h>
#include <AUI/Test/UI/Assertion/Color.h>
#include <AUI/Test/UI/Assertion/ParentOf.h>
