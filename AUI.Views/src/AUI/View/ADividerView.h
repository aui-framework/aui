//
// Created by alex2 on 28.09.2020.
//

#pragma once


#include "AView.h"

class API_AUI_VIEWS ADividerView: public AView {
public:
    ADividerView();
    ~ADividerView() override;

    bool consumesClick(const glm::ivec2& pos) override;
};


