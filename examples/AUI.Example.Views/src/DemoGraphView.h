#pragma once


#include "AUI/View/AView.h"

class DemoGraphView: public AView {
public:
    DemoGraphView();

    void render() override;

private:
    AVector<glm::vec2> mPoints;
};


