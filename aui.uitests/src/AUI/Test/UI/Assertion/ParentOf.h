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
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>
#include "Traits.h"

struct ViewAssertionParentOf {
    using IGNORE_VISIBILITY = std::true_type;

    _<AView> checkedParent;

    explicit ViewAssertionParentOf(_<AView> checkedParent) : checkedParent(std::move(checkedParent)) {}

    bool operator()(const _<AView>& v) {
        return checkedParent->hasIndirectParent(v);
    }
};


using isParentOf = ViewAssertionParentOf;