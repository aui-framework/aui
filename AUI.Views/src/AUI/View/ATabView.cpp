//
// Created by alex2 on 24.10.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "ATabView.h"

class ATabButtonView: public ALabel {
private:
    bool mCurrent = false;

public:
    ATabButtonView(const AString& text): ALabel(text) {
        AVIEW_CSS;
    }

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override {
        AView::getCustomCssAttributes(map);
        if (mCurrent)
            map["current"] = true;
    }

    void setCurrent(bool current) {
        mCurrent = current;
        emit customCssPropertyChanged;
    }
};

ATabView::ATabView() {
    setLayout(_new<AVerticalLayout>());

    addView(mButtonsRow = _container<AHorizontalLayout>({}));
    addView(mPageView = _new<APageView>());

    mButtonsRow->addCssName(".tabview_row");

    connect(mPageView->pageChanging, this, [&](unsigned pageId) {
        _cast<ATabButtonView>(mButtonsRow->getViews()[mPageView->getPageId()])->setCurrent(false);
        _cast<ATabButtonView>(mButtonsRow->getViews()[pageId])->setCurrent(true);
    });
}

void ATabView::addTab(const _<AView>& view, const AString& name) {
    mPageView->addPage(view);
    auto id = mButtonsRow->getViews().size();
    auto c = _new<ATabButtonView>(name);
    c->setCurrent(id == mPageView->getPageId());
    mButtonsRow->addView(c);

    connect(c->clicked, this, [&, id]() {
       setTabId(id);
    });
}

void ATabView::setTabId(unsigned int tabId) {
    mPageView->setPageId(tabId);
}
