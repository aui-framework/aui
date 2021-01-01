#pragma once

#include <AUI/ASS/Declaration/Selector.h>

struct Rule {
public:
    template<typename... Declarations>
    Rule(ass::ASelector&& selector, Declarations&&... declarations):
            mSelector(std::forward<ass::ASelector>(selector))
        {
        processDeclarations(std::forward<Declarations>(declarations)...);
    }

    [[nodiscard]] const AVector<ass::decl::IDeclarationBase*>& getDeclarations() const {
        return mDeclarations;
    }

    [[nodiscard]] const ass::ASelector& getSelector() const {
        return mSelector;
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

    ass::ASelector mSelector;
    AVector<ass::decl::IDeclarationBase*> mDeclarations;
};