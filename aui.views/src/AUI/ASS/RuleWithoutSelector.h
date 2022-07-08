/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include "Declaration/IDeclaration.h"

struct RuleWithoutSelector: aui::noncopyable {
public:
    template<typename... Declarations>
    RuleWithoutSelector(Declarations&&... declarations)
    {
        processDeclarations(std::forward<Declarations>(declarations)...);
    }

    RuleWithoutSelector() {

    }

    [[nodiscard]] const AVector<std::unique_ptr<ass::decl::IDeclarationBase>>& getDeclarations() const noexcept {
        return mDeclarations;
    }

    void addDeclaration(std::unique_ptr<ass::decl::IDeclarationBase> declaration) {
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

            mDeclarations.emplace_back(std::make_unique<declaration_t>(t));
        }
    }

    AVector<std::unique_ptr<ass::decl::IDeclarationBase>> mDeclarations;
};