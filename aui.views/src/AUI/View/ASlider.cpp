/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 3/23/2023.
//

#include "ASlider.h"
#include "AUI/Common/AObject.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AProgressBar.h"

using namespace ass;
using namespace declarative;


namespace {
class SliderHandleWrapper: public AViewContainerBase {
public:
    SliderHandleWrapper(_<AView> handle): mHandle(Centered { std::move(handle) }) {
        addView(mHandle);
    }

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
        AViewContainerBase::onPointerMove(pos, event);
        if (!isPressed(event.pointerIndex)) {
            return;
        }
        emit valueChanged(pos.x / float(getSize().x));
    }

    void onPointerPressed(const APointerPressedEvent& event) override {
        AViewContainerBase::onPointerPressed(event);
        emit valueChanged(event.position.x / float(getSize().x));
    }

    bool consumesClick(const glm::ivec2& pos) override {
        return true;
    }

    void setValue(aui::float_within_0_1 value) {
        mValue = value;
        markMinContentSizeInvalid();
    }

    int getMinimumWidth() override {
        return mHandle->getMinimumWidth();
    }

    int getMinimumHeight() override {
        return mHandle->getMinimumHeight();
    }

    emits<aui::float_within_0_1> valueChanged;

protected:
    void applyGeometryToChildren() override {
        auto handleSize = mHandle->getMinimumSize();
        mHandle->setGeometry({getSize().x * mValue - handleSize.x / 2, 0}, { handleSize.x, getSize().y });
    }

private:
    _<AView> mHandle;
    aui::float_within_0_1 mValue = 0;
};

}

/// [defaultTrack]
_<AView> Slider::defaultTrack(const contract::In<aui::float_within_0_1>& value) {
    return ProgressBar { .progress = value } AUI_WITH_STYLE {
        FixedSize { {}, 4_dp },
        MinSize { 150_dp, {} },
    };
}
/// [defaultTrack]

/// [defaultHandle]
_<AView> Slider::defaultHandle() {
    return _new<AView>() AUI_WITH_STYLE {
        BackgroundSolid { AStylesheet::getOsThemeColor() },
        FixedSize { 8_dp },
        BorderRadius { 4_dp },
    };
}
/// [defaultHandle]

API_AUI_VIEWS _<AView> Slider::operator()() {
    auto handleWrapper = _new<SliderHandleWrapper>(std::move(handle));
    value.bindToCopy(ASlotDef{AUI_SLOT(handleWrapper.get())::setValue});
    onValueChanged.bindTo(handleWrapper->valueChanged);
    track->setExpanding({1, 0});
    return Stacked {
        std::move(track),
        std::move(handleWrapper) AUI_WITH_STYLE { Expanding{} },
    };
}
