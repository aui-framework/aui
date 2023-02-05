// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <AUI/Common/AString.h>
#include "UIMatcher.h"

/**
 * @brief Matcher factory namespace.
 * @ingroup uitests
 */
namespace By {
    API_AUI_UITESTS UIMatcher text(const AString& text);
    API_AUI_UITESTS UIMatcher name(const AString& name);

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


