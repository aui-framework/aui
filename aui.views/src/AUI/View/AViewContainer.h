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

#include "AViewContainerBase.h"

/**
 * @brief A trivial modifiable view that represents a set of views.
 * @details
 * AViewContainer can store, render, resize, provide events to and handle the child views.
 *
 * AViewContainer does not control the position and size of the child views by itself; instead, it delegates that
 * responsibility to it's [layout manager](layout-managers.md).
 *
 * Since AViewContainer is an instance of AView, AViewContainer can handle AViewContainers recursively, thus, making
 * possible complex UI by nested AViewContainers with different layout managers.
 *
 * Consider using AViewContainerBase as a base class when possible. Use AViewContainer if you want to expose view
 * modifying methods such as addView, removeView, setContents, setLayout, etc. Use AViewContainerBase if these methods
 * might cause unwanted interference with implementation details of your view.
 */
class API_AUI_VIEWS AViewContainer: public AViewContainerBase {
public:
    AViewContainer();
    ~AViewContainer() override;

    using AViewContainerBase::addView;
    using AViewContainerBase::addViews;
    using AViewContainerBase::addViewCustomLayout;
    using AViewContainerBase::removeView;
    using AViewContainerBase::removeAllViews;
    using AViewContainerBase::setViews;
    using AViewContainerBase::setContents;
    using AViewContainerBase::getLayout;
    using AViewContainerBase::setLayout;
};
