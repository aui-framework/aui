// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#pragma once


#include "AViewContainer.h"
#include "AProgressBar.h"

/**
 * @brief Slider control.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ASlider: public AViewContainer {
public:
    class Handle: public AView {}; // embed class for styling

    ASlider();
    void onPointerMove(glm::ivec2 pos) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void updateLayout() override;
    bool capturesFocus() override;


    void setValue(aui::float_within_0_1 value) {
        mProgress->setValue(value);
    }

    [[nodiscard]]
    aui::float_within_0_1 value() const noexcept {
        return mProgress->value();
    }

    [[nodiscard]]
    const _<Handle>& handle() const noexcept {
        return mHandle;
    }

    [[nodiscard]]
    const _<AProgressBar>& progressbar() const noexcept {
        return mProgress;
    }

signals:
    emits<aui::float_within_0_1> valueChanging;
    emits<aui::float_within_0_1> valueChanged;

private:
    _<Handle> mHandle;
    _<AProgressBar> mProgress;
    bool mDragging = false;

    void updateSliderWithPosition(glm::ivec2 pointerPosition);

    void updateHandlePosition();
};
