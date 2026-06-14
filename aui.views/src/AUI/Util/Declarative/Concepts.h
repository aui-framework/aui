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

#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/parameter_pack.h>
#include <AUI/ASS/ASS.h>

namespace aui::ui_building {

/**
 * @brief Shared pointer to AView alias.
 */
using View = _<AView>;

/**
 * @brief List of AView's. Like a container, but not assigned to any layout yet.
 */
using ViewGroup = AVector<_<AView>>;

/**
 * @brief Either View or ViewGroup.
 */
using ViewOrViewGroup = std::variant<View, ViewGroup>;

/**
 * @brief Shared pointer to AViewContainer alias.
 */
using ViewContainer = _<AViewContainer>;

/**
 * @brief Denotes any shared pointer to a subclass of AView.
 */
template <typename T>
concept LayoutItemView = aui::convertible_to<T, View>;

/**
 * @brief Denotes anything that can be converted to ViewGroup, i.e, std::initializer_list<_<AView>>.
 */
template <typename T>
concept LayoutItemViewGroup = aui::convertible_to<T, ViewGroup>;

/**
 * @brief Denotes any callable that return a View. The callable is typically invoked just in place.
 */
template <typename T>
concept LayoutItemViewFactory = aui::factory<T, View>;

template <typename Item>
concept LayoutItemAny = LayoutItemView<Item> || LayoutItemViewGroup<Item> || LayoutItemViewFactory<Item>;
}
