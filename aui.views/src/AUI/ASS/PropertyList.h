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

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include "Property/IProperty.h"
#include <AUI/Common/AObject.h>
#include <AUI/Traits/types.h>


namespace ass {

    struct PropertyList {
    public:
        template<typename... Declarations>
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
