//
// Created by Alex2772 on 12/5/2021.
//


#include "Matcher.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowManager.h>

ASet<_<AView>> Matcher::toSet() const {
    ASet<_<AView>> result;

    for (auto& window : ABaseWindow::getWindowManager().getWindows()) {
        processContainer(result, window);
    }
    return result;
}

void Matcher::processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const {
    for (auto& view : container) {
        if (view->getVisibility() == Visibility::VISIBLE && mMatcher->matches(view)) {
            destination << view;
        }
        if (auto currentContainer = _cast<AViewContainer>(view)) {
            processContainer(destination, currentContainer);
        }
    }
}
