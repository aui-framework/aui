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

#include <AUI/Common/AString.h>
#include "UIMatcher.h"

/**
 * @brief Matcher factory namespace.
 * @ingroup uitests
 */
namespace By {
    API_AUI_UITESTS UIMatcher text(const AString& text);
    API_AUI_UITESTS UIMatcher name(const AString& name);
    API_AUI_UITESTS UIMatcher value(const _<AView>& value);

    template<typename T>
    UIMatcher type() {
        class TypeMatcher: public IMatcher {
        public:
            ~TypeMatcher() override = default;

            bool matches(const _<AView>& view) override {
                return dynamic_cast<T*>(view.get()) != nullptr;
            }
        };
        return { _new<TypeMatcher>() };
    }
    template<typename T>
    UIMatcher exactType() {
        class ExactTypeMatcher: public IMatcher {
        public:
            ~ExactTypeMatcher() override = default;

            bool matches(const _<AView>& view) override {
                return typeid(*view.get()) == typeid(T);
            }
        };
        return { _new<ExactTypeMatcher>() };
    }
}


