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
// Created by alex2 on 03.01.2021.
//

#pragma once

#include "PropertyList.h"
#include "AUI/ASS/Selector/AAssSelector.h"


namespace ass {

    struct PropertyListConditional;

    template<typename T>
    concept ValidDeclarationOrPropertyList = ValidDeclaration<T> || aui::derived_from<T, PropertyListConditional>;

    struct PropertyListRecursive: public PropertyList {
    public:
        template<ValidDeclarationOrPropertyList... Declarations>
        PropertyListRecursive(Declarations&& ... declarations): PropertyListRecursive() {
            processDeclarations(std::forward<Declarations>(declarations)...);
        }

        PropertyListRecursive(const PropertyListRecursive&);
        PropertyListRecursive();
        ~PropertyListRecursive();


        PropertyListRecursive(const PropertyList& p);
        PropertyListRecursive(PropertyList&& p);

        [[nodiscard]]
        const AVector<PropertyListConditional>& conditionalPropertyLists() const noexcept {
            return mConditionalPropertyLists;
        }

    private:
        AVector<PropertyListConditional> mConditionalPropertyLists;

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

    struct PropertyListConditional {
        AAssSelector selector;
        PropertyListRecursive list;

        template<typename... Declarations>
        PropertyListConditional(AAssSelector selector, Declarations&&... declarations):
                selector(std::move(selector)), list(std::forward<Declarations>(declarations)...)
        {}
    };

    inline PropertyListRecursive::PropertyListRecursive() = default;
    inline PropertyListRecursive::PropertyListRecursive(const PropertyList& p) : PropertyList(p) {}
    inline PropertyListRecursive::PropertyListRecursive(PropertyList&& p) : PropertyList(std::move(p)) {}
    inline PropertyListRecursive::PropertyListRecursive(const PropertyListRecursive&) = default;
    inline PropertyListRecursive::~PropertyListRecursive() = default;


    template<typename T>
    void PropertyListRecursive::processDeclaration(T&& t) {
        if constexpr (std::is_base_of_v<PropertyListConditional, T>) {
            mConditionalPropertyLists << std::forward<PropertyListConditional&&>(t);
        } else if constexpr (std::is_same_v<T, PropertyListRecursive>) {
            // aka move constructor
            mProperties = std::move(t.mProperties);
            mConditionalPropertyLists = std::move(t.mConditionalPropertyLists);
        } else {
            using declaration_t = ass::prop::Property<std::decay_t<T>>;
            static_assert(aui::is_complete<declaration_t>,
                          "ass::prop::Property template specialization is not defined for this property");

            mProperties << _new<declaration_t>(t);
        }
    }
}
