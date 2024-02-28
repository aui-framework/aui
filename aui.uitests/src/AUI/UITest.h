// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
