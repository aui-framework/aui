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
#include <AUI/Util/ADataBinding.h>

/**
 * @brief A progress bar.
 *
 * ![](imgs/views/AProgressBar.png)
 *
 * @ingroup views_indication
 * @details
 * A progress bar is used to express a long operation (i.e. file copy) with known progress and reassure the user that
 * application is still running.
 */
class API_AUI_VIEWS AProgressBar: public AViewContainerBase {
public:
    class Inner: public AView {
    public:
        ~Inner() override;
    };
    ~AProgressBar() override;

    /**
     * Set progress bar value.
     * @param value progress value, where `0.0f` = 0%, `1.0f` = 100%
     */
    void setValue(aui::float_within_0_1 value) {
        mValue = value;
        updateInnerWidth();
        redraw();

        emit mValueChanged(value);
    }

    [[nodiscard]]
    auto value() const noexcept {
        return APropertyDef {
            this,
            &AProgressBar::mValue,
            &AProgressBar::setValue,
            mValueChanged,
        };
    }

    [[nodiscard]]
    const _<Inner>& innerView() const noexcept {
        return mInner;
    }

public:
    AProgressBar();

    void setSize(glm::ivec2 size) override;
    void render(ARenderContext context) override;

private:
    aui::float_within_0_1 mValue = 0.f;
    emits<aui::float_within_0_1> mValueChanged;
    void updateInnerWidth();
    _<Inner> mInner;
};

template<>
struct ADataBindingDefault<AProgressBar, aui::float_within_0_1> {
public:
    static auto property(const _<AProgressBar>& view) {
        return view->value();
    }
    static void setup(const _<AProgressBar>& view) {}
};
