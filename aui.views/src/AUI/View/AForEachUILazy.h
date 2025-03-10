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

#include "AForEachUIBase.h"

/**
 * @brief Version of @ref AUI_DECLARATIVE_FOR_BASE with lazy layout semantics.
 * @ingroup useful_views
 * @details
 * @experimental
 *
 * Allows lazy semantics in regards of view instansiating and layout processing inside the scroll area environment. That
 * is, only visible views will be processed, saving CPU and memory especially with large (or potentially infinite)
 * amounts of contents.
 *
 * Possible use cases are: social media feed, messages lists.
 *
 * As a downside, scroll area's content size can't be predicted properly. This mostly affects scroll bars only,
 * which is considered as small cost compared to performance benefits achieved by lazy layout semantics. With
 * potentially infinite contents, scroll bars are considered useless anyway.
 *
 * AUI, however, does its best by making certain assumptions based on current scroll position, current content
 * size and available data (in case of AUI_DECLARATIVE_FOR) to predict actual content size, to make scroll bars
 * as smooth as possible.
 */
template<typename T, typename Layout>
class API_AUI_VIEWS AForEachUILazy : public AForEachUIBase<T, Layout> {
public:
  using AForEachUIBase<T, Layout>::AForEachUIBase;
    ~AForEachUILazy() override = default;

};

#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...) _new<AForEachUILazy<std::decay_t<decltype(model)>::stored_t::stored_t, layout>>(model) - [__VA_ARGS__](const std::decay_t<decltype(model)>::stored_t::stored_t& value, size_t index) -> _<AView>
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
