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
// Created by alex2 on 03.01.2021.
//

#pragma once

#include "Property/IProperty.h"
#include <AUI/Common/AObject.h>
#include <AUI/Traits/types.h>


namespace ass {

    template<typename T>
    concept ValidDeclaration = aui::is_complete<ass::prop::Property<std::decay_t<T>>>;

    struct PropertyList {
    public:
        template<ValidDeclaration... Declarations>
        PropertyList(Declarations&& ... declarations) {
            processDeclarations(std::forward<Declarations>(declarations)...);
        }

        PropertyList() {

        }

        [[nodiscard]] const AVector<_<ass::prop::IPropertyBase>>& declarations() const noexcept {
            return mProperties;
        }

        void addDeclaration(_<ass::prop::IPropertyBase> declaration) {
            mProperties << std::move(declaration);
        }

    protected:
        template<typename Property, typename... Declarations>
        void processDeclarations(Property&& declaration, Declarations&& ... declarations) {
            processDeclaration(std::forward<Property>(declaration));
            if constexpr (sizeof...(Declarations) > 0) {
                processDeclarations(std::forward<Declarations>(declarations)...);
            }
        }

        template<typename T>
        void processDeclaration(T&& t) {
            if constexpr (std::is_same_v<T, PropertyList>) {
                mProperties = std::move(t.mProperties);
            } else {
                using declaration_t = ass::prop::Property<std::decay_t<T>>;
                static_assert(aui::is_complete<declaration_t>,
                              "ass::prop::Property template specialization is not defined for this declaration");

                mProperties << _new<declaration_t>(t);
            }
        }

        AVector<_<ass::prop::IPropertyBase>> mProperties;
    };
}
