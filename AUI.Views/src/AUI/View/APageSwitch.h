//
// Created by alex2 on 27.10.2020.
//

#pragma once


#include "ALabel.h"
#include "APageView.h"

class API_AUI_VIEWS APageSwitch: public ALabel {
private:
    unsigned mPageNumber;
    _<APageView> mPageView;

public:
    APageSwitch(const AString& text, unsigned pageNumber, const _<APageView>& pageView);

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;

    int getContentMinimumWidth() override;
};


