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

#include <AUI/View/AViewContainer.h>
#include <AUI/Traits/values.h>

/**
 * @brief A progress bar.
 * @ingroup useful_views
 * @details
 * A progress bar is used to express a long operation (i.e. file copy) with known progress and reassure the user that
 * application is still running.
 */
class API_AUI_VIEWS AProgressBar: public AViewContainer {
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
    AProgressBar();

    void setSize(glm::ivec2 size) override;
    void render(ClipOptimizationContext context) override;

private:
    aui::float_within_0_1 mValue = 0.f;
    void updateInnerWidth();
    _<Inner> mInner;


signals:
    emits<aui::float_within_0_1> valueChanged;
};


