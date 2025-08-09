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


#include "AView.h"
#include "AUI/Common/ATimer.h"

/**
 * @brief Circular infinite progress indicator.
 * @ingroup useful_views
 * @details
 * ![](imgs/Screenshot_20250205_061349.png)
 *
 * Rotates its contents as it renders. Unlike ASpinner, ASpinnerV2 allows to take more control over the animation -
 * ASpinnerV2 draws at lower framerate which is better from perspective of resource usage. Generally, use ASpinnerV2
 * instead of ASpinner.
 *
 * ASpinner's default style is a circular arc constantly and smoothly rotating around. ASpinnerV2's default style is a
 * segmented circle which visually substains the "stepness" of the animation.
 *
 * ASpinnerV2 is configured through [ass] :
 * <!-- aui:snippet aui.views/src/AUI/ASS/AStylesheet.cpp ASpinnerV2 -->
 */
class API_AUI_VIEWS ASpinnerV2: public AView {
public:
    ASpinnerV2();
    ~ASpinnerV2() override = default;
    void render(ARenderContext ctx) override;

    struct Configuration {
        std::chrono::milliseconds period = std::chrono::seconds(1);
        size_t steps = 1;
    };


    void setConfiguration(Configuration configuration);

private:
    _<ATimer> mTimer;
    Configuration mConfiguration;
};

template<>
struct API_AUI_VIEWS ass::prop::Property<ASpinnerV2::Configuration>: ass::prop::IPropertyBase {
public:
    Property(const ASpinnerV2::Configuration& configuration) : mConfiguration(configuration) {}
    ~Property() override = default;

    void applyFor(AView *view) override;
private:
    ASpinnerV2::Configuration mConfiguration;
};
