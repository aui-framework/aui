//
// Created by Alex2772 on 12/5/2021.
//


#include "UIMatcher.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowManager.h>

ASet<_<AView>> UIMatcher::toSet() const {
    ASet<_<AView>> result;

    for (auto& window : ABaseWindow::getWindowManager().getWindows()) {
        processContainer(result, window);
    }
    return result;
}

UIMatcher*& UIMatcher::currentImpl() {
    thread_local UIMatcher* matcher;
    return matcher;
}

void UIMatcher::processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const {
    for (auto& view : container) {
        if (mIncludeInvisibleViews || view->getVisibility() == Visibility::VISIBLE) {
            if (mMatcher->matches(view)) {
                destination << view;
            }
            if (auto currentContainer = _cast<AViewContainer>(view)) {
                processContainer(destination, currentContainer);
            }
        }
    }
}
