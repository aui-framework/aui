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
#include "AUI/Util/Declarative/Contracts.h"
#include <functional>

/**
 * ---
 * title: Progress indicators
 * icon: material/progress
 * ---
 *
 * @brief Progress bars show the progress of an operation.
 *
 * ![](imgs/views/AProgressBar.png)
 *
 * @ingroup views_indication
 * @details
 * Progress bars are visual indicators that show the completion progress of an operation, process, or task.
 * Progress bars provide users with real-time feedback on progress of a defined operation.
 *
 * Consider these three use cases where you might use a progress indicator:
 *
 * - **Loading content**: While fetching content from a network, such as loading an image or data for a user profile.
 * - **File upload**: Give the user feedback on how long the upload might take.
 * - **Long processing**: While an app is processing a large amount of data, convey to the user how much of the total is
 *   complete.
 *
 * ## Creating a basic progress bar
 *
 * The following code snippet shows a minimal progress bar implementation:
 *
 * <!-- aui:snippet examples/ui/progressbar/src/main.cpp AProgressBar_example -->
 *
 * ### Key points about this code
 *
 * - `struct State` holds a reactive property `progress` representing the progress bar's value. When `progress` changes,
 *    the UI updates reactively.
 * - The `ProgressBar` view binds `.value` to the state's property, so the progress is kept in sync with the data.
 * - Value ranges from `0.0f` (0%) to `1.0f` (100%).
 * - The UI updates automatically based on the state because of AUI's reactive system.
 *
 * ## Styling
 *
 * Both `AProgressBar` and `AProgressBar::Inner` are exposed for styling purposes.
 *
 * <!-- aui:snippet aui.views/src/AUI/ASS/AStylesheet.cpp AProgressBar -->
 */
class API_AUI_VIEWS AProgressBar : public AViewContainerBase {
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
    const _<AView>& innerView() const noexcept {
        return mInner;
    }

    /**
     * Set custom inner view.
     * @param inner custom inner view to use instead of default one
     */
    void setInnerView(_<AView> inner) {
        if (mInner) {
            removeView(mInner);
        }
        mInner = std::move(inner);
        addView(mInner);
        mInner->setPosition({0, 0});
        updateInnerWidth();
    }

public:
    AProgressBar();

    void onLayout(int w, int h) override;

private:
    aui::float_within_0_1 mValue = 0.f;
    emits<aui::float_within_0_1> mValueChanged;
    void updateInnerWidth();
    _<AView> mInner;
};

namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct ProgressBar {
    /**
     * @brief Current progress value.
     * @details
     * Where `0.0f` = 0%, `1.0f` = 100%
     */
    contract::In<aui::float_within_0_1> progress;

    /**
     * @brief Function that creates the progress bar fill view.
     * @details
     * By default this is `defaultInner` which creates a simple rectangular fill.
     * Users can replace this with a custom function to change the appearance
     * of the progress bar fill. The function  should return a view that visually represents the progress.
     *
     * View's width will be automatically adjusted based on the `progress` value, so the view should be designed to look
     * good at various widths.
     */
    _<AView> inner = defaultInner();

    /**
     * @brief Creates a default inner view for the progress bar.
     * @return A view representing the progress bar fill.
     * @details
     * The returned view is a simple rectangle that visually represents
     * the progress bar's fill area.
     */
    static _<AView> defaultInner();

    API_AUI_VIEWS _<AView> operator()();
};
}   // namespace declarative

// legacy

template<>
struct ADataBindingDefault<AProgressBar, aui::float_within_0_1> {
public:
    static auto property(const _<AProgressBar>& view) {
        return view->value();
    }
    static void setup(const _<AProgressBar>& view) {}
};
