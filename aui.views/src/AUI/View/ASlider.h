/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include "AViewContainer.h"
#include "AProgressBar.h"

/**
 * @brief Slider control.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ASlider: public AViewContainerBase {
public:
    class Handle: public AView {}; // embed class for styling

    ASlider();
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void applyGeometryToChildren() override;
    bool capturesFocus() override;

    [[nodiscard]]
    bool isDragging() const noexcept {
        return isPressed();
    }

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

    void updateSliderWithPosition(glm::ivec2 pointerPosition);

    void updateHandlePosition();
};
