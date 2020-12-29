//
// Created by alex2 on 24.10.2020.
//

#pragma once

#include "APageView.h"

class API_AUI_VIEWS ATabView: public AViewContainer {
private:
    _<APageView> mPageView;
    _<AViewContainer> mButtonsRow;

public:
    ATabView();

    void addTab(const _<AView>& view, const AString& name = {});
    void setTabId(unsigned tabId);
};


