//
// Created by alex2 on 27.10.2020.
//

#include "APageSwitch.h"

APageSwitch::APageSwitch(const AString& text, unsigned pageNumber, const _<APageView>& pageView):
    ALabel(text),
    mPageNumber(pageNumber),
    mPageView(pageView)
{
    AVIEW_CSS;
    connect(pageView->pageChanged, this, [&]() {
        emit customCssPropertyChanged();
    });
    connect(clicked, this, [pageView, pageNumber]() {
        pageView->setPageId(pageNumber);
    });
}

void APageSwitch::getCustomCssAttributes(AMap<AString, AVariant>& map) {
    AView::getCustomCssAttributes(map);
    if (mPageView->getPageId() == mPageNumber) {
        map["current"] = true;
    }
}

int APageSwitch::getContentMinimumWidth() {
    return ALabel::getContentMinimumWidth();
}
