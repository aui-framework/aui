//
// Created by alex2 on 24.10.2020.
//

#pragma once


#include "AViewContainer.h"

class API_AUI_VIEWS APageView: public AViewContainer {
private:
    unsigned mPageId = 0;

public:
    APageView();

    void addPage(const _<AView>& view);
    void setPageId(unsigned int pageId);

    unsigned getPageId() const {
        return mPageId;
    }

signals:
    emits<unsigned> pageChanging;
    emits<> pageChanged;

};


