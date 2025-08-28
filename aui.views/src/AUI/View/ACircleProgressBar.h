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

#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/Traits/values.h>

/**
 * @brief A circle-shaped progress bar.
 *
 * ![](imgs/views/ACircleProgressBar.png)
 *
 * @ingroup views_indication
 * @details
 * A progress bar is used to express a long operation (i.e. file copy) with known progress and reassure the user that
 * application is still running.
 *
 * Unlike AProgressBar, this one is circle shaped; thus occupies less space.
 */
class API_AUI_VIEWS ACircleProgressBar: public AViewContainerBase {
public:
    class Inner: public AView {
    public:
        ~Inner() override;
    };
    ~ACircleProgressBar() override;

    /**
     * Set progress bar value.
     * @param value progress value, where `0.0f` = 0%, `1.0f` = 100%
     */
    void setValue(aui::float_within_0_1 value) {
        mValue = value;
        redraw();

        emit valueChanged(value);
    }

    [[nodiscard]]
    aui::float_within_0_1 value() const noexcept {
        return mValue;
    }

    [[nodiscard]]
    const _<Inner>& innerView() const noexcept {
        return mInner;
    }

public:
    ACircleProgressBar();

    void render(ARenderContext context) override;

private:
    aui::float_within_0_1 mValue = 0.f;
    _<Inner> mInner;


signals:
    emits<aui::float_within_0_1> valueChanged;
};


