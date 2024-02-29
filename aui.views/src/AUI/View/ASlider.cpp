// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 3/23/2023.
//

#include "ASlider.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AProgressBar.h"

using namespace ass;

ASlider::ASlider() {
    setContents(Stacked {
        mProgress = _new<AProgressBar>() with_style {
            Expanding{1, 0},
        },
        mHandle = _new<Handle>()
    });

    connect(mProgress->valueChanged, [this](aui::float_within_0_1 v) {
        emit valueChanging(v); // we would emit ed signal in pointer release method
        updateHandlePosition();
    });

    setValue(0.f);
}

void ASlider::onPointerPressed(const APointerPressedEvent& event) {
    AView::onPointerPressed(event); // we do not want to gain focus to some of our childs; so calling AView directly
    updateSliderWithPosition(event.position);
}

void ASlider::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, event);
    if (isDragging()) {
        updateSliderWithPosition(pos);
    }
}

void ASlider::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    updateSliderWithPosition(event.position);

    emit valueChanged(value());
}

void ASlider::updateSliderWithPosition(glm::ivec2 pointerPosition) {
    setValue(float(pointerPosition.x) / float(getContentWidth()));
}

void ASlider::updateLayout() {
    AViewContainer::updateLayout();
    updateHandlePosition();
}

void ASlider::updateHandlePosition() {
    const auto& progressInner = mProgress->innerView();
    const auto progressRightCornerAbsolutePos = progressInner->getPositionInWindow().x + progressInner->getWidth();
    const auto handleCenteringOffset = mHandle->getWidth() / 2;
    mHandle->setPosition({getPadding().left + progressRightCornerAbsolutePos - getPositionInWindow().x - handleCenteringOffset,
                          mHandle->getPosition().y });
}

bool ASlider::capturesFocus() {
    return true; // we want to receive onPointerMove events even when pointer is outside
}
