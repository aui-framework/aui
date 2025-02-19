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

#include "AScrollArea.h"

/**
 * @brief Version of AScrollArea with lazy layout semantics.
 * @ingroup useful_views
 * @details
 * @experimental
 *
 * Allows lazy semantics in regards of layout processing inside the scroll area. That is, only visible view's layout
 * will be processed, saving CPU and memory especially with large (or potentially infinite) amounts of contents.
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
class API_AUI_VIEWS AScrollAreaLazy : public AScrollArea {
public:
    ~AScrollAreaLazy() override = default;
    class Builder : public AScrollArea::Builder {
        friend class AScrollArea;

    public:
        Builder() = default;

        _<AScrollAreaLazy> build() { return aui::ptr::manage(new AScrollAreaLazy(*this)); }

        operator _<AView>() { return build(); }
        operator _<AViewContainerBase>() { return build(); }
    };

private:
    AScrollAreaLazy(Builder& builder);
};