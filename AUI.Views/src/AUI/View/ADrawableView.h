//
// Created by alex2 on 23.10.2020.
//

#pragma once

#include <AUI/Image/IDrawable.h>
#include "AView.h"

class API_AUI_VIEWS ADrawableView: public AView {
private:
    _<IDrawable> mDrawable;
    AColor mColorOverlay;

public:
    ADrawableView(const _<IDrawable>& drawable);
    void render() override;

protected:
    void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;

};


