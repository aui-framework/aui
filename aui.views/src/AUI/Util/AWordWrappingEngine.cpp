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

//
// Created by Alex2772 on 11/16/2021.
//

#include <range/v3/all.hpp>
#include <AUI/Traits/iterators.h>
#include "AWordWrappingEngine.h"
#include <algorithm>
#include <numeric>
#include <AUI/Util/AFraction.h>



AFloat AWordWrappingEngineBase::Entry::getFloat() const {
    return AFloat::NONE;
}

bool AWordWrappingEngineBase::Entry::forcesNextLine() const {
    return false;
}
