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

#include "PropertyList.h"
#include "AUI/ASS/Selector/AAssSelector.h"


namespace ass {

    struct PropertyListRecursive: public PropertyList {
    public:
        struct ConditionalPropertyList;

        template<typename... Declarations>
        PropertyListRecursive(Declarations&& ... declarations) {
            processDeclarations(std::forward<Declarations>(declarations)...);
        }

        PropertyListRecursive() {

        }

        PropertyListRecursive(const PropertyList& p) : PropertyList(p) {}
        PropertyListRecursive(PropertyList&& p) : PropertyList(std::move(p)) {}


        [[nodiscard]]
        const AVector<ConditionalPropertyList>& conditionalPropertyLists() const noexcept {
            return mConditionalPropertyLists;
        }

    private:
        AVector<ConditionalPropertyList> mConditionalPropertyLists;

        template<typename Property, typename... Declarations>
        void processDeclarations(Property&& declaration, Declarations&& ... declarations) {
            processDeclaration(std::forward<Property>(declaration));
            if constexpr (sizeof...(Declarations) > 0) {
                processDeclarations(std::forward<Declarations>(declarations)...);
            }
        }

        template<typename T>
        void processDeclaration(T&& t);

    };

    struct PropertyListRecursive::ConditionalPropertyList {
        AAssSelector selector;
        PropertyListRecursive list;
    };


    template<typename T>
    void PropertyListRecursive::processDeclaration(T&& t) {
        if constexpr (std::is_base_of_v<PropertyListRecursive::ConditionalPropertyList, T>) {
            mConditionalPropertyLists << std::forward<ConditionalPropertyList&&>(t);
        } else {
            using declaration_t = ass::prop::Property<std::decay_t<T>>;
            static_assert(aui::is_complete<declaration_t>,
                          "ass::prop::Property template specialization is not defined for this property");

            mProperties << _new<declaration_t>(t);
        }
    }
}
