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

namespace uitest::impl {
    template<typename L, typename R>
    struct both: L, R {
        bool operator()(const _<AView>& v) {
            return L::operator()(v) && R::operator()(v);
        }
    };

    template<typename P>
    struct not$: P {
        explicit not$(P p): P(std::move(p)) {}

        bool operator()(const _<AView>& v) {
            return !P::operator()(v);
        }
    };
}
