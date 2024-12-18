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

#include <memory>
#include <cstring>

namespace aui {
    /**
     * @brief Fill with zeros object's memory.
     * @tparam T object type
     * @param data object itself
     */
    template<typename T>
    inline void zero(T& data) {
        memset(&data, 0, sizeof(T));
    }
}