#pragma once

#include "IDeclaration.h"

struct Rule {
public:
    template<typename... Declarations>
    explicit Rule(Declarations&&... declarations) {
        processDeclarations(std::forward<Declarations>(declarations)...);
    }

    const AVector<ass::decl::IDeclarationBase*>& getDeclarations() const {
        return mDeclarations;
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
        mDeclarations.emplace_back(new ass::decl::Declaration<T>(t));
    }

    AVector<ass::decl::IDeclarationBase*> mDeclarations;
};