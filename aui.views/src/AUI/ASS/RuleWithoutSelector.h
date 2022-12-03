// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include "Declaration/IDeclaration.h"

struct RuleWithoutSelector {
public:
    template<typename... Declarations>
    RuleWithoutSelector(Declarations&&... declarations)
    {
        processDeclarations(std::forward<Declarations>(declarations)...);
    }

    RuleWithoutSelector() {

    }

    [[nodiscard]] const AVector<_<ass::decl::IDeclarationBase>>& getDeclarations() const noexcept {
        return mDeclarations;
    }

    void addDeclaration(_<ass::decl::IDeclarationBase> declaration) {
        mDeclarations << std::move(declaration);
    }

private:
    template<typename Declaration, typename... Declarations>
    void processDeclarations(Declaration&& declaration, Declarations&&... declarations) {
        processDeclaration(std::forward<Declaration>(declaration));
        if constexpr (sizeof...(Declarations) > 0) {
            processDeclarations(std::forward<Declarations>(declarations)...);
        }
    }

    template<typename T>
    void processDeclaration(T&& t) {
        if constexpr(std::is_same_v<T, RuleWithoutSelector>) {
            mDeclarations = std::move(t.mDeclarations);
        } else {
            using declaration_t = ass::decl::Declaration<std::decay_t<T>>;
            static_assert(aui::is_complete<declaration_t>,
                          "ass::decl::Declaration template specialization is not defined for this declaration");

            mDeclarations << _new<declaration_t>(t);
        }
    }

    AVector<_<ass::decl::IDeclarationBase>> mDeclarations;
};