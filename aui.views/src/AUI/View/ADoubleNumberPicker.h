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

#include <AUI/Traits/values.h>
#include <AUI/Util/ADataBinding.h>

#include "AAbstractTextField.h"
#include "AViewContainer.h"

/**
 * @brief A text field for numbers with increase/decrease buttons.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ADoubleNumberPicker : public AViewContainer {
   private:
    class ADoubleNumberPickerField : public AAbstractTextField {
       private:
        ADoubleNumberPicker& mPicker;

       public:
        ADoubleNumberPickerField(::ADoubleNumberPicker& picker) : mPicker(picker) {}

        virtual ~ADoubleNumberPickerField() = default;

        void onKeyRepeat(AInput::Key key) override;

       protected:
        bool isValidText(const AString& text) override;
    };

    _<ADoubleNumberPickerField> mTextField;

    double mMin = 0;
    double mMax = 100;

   public:
    ADoubleNumberPicker();

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void setValue(double v);

    [[nodiscard]] const AString& text() const noexcept { return mTextField->text(); }

    [[nodiscard]] double getValue() const { return mTextField->text().toDouble().valueOr(0.0); }

    [[nodiscard]] double getMin() const { return mMin; }

    [[nodiscard]] double getMax() const { return mMax; }

    void setMin(double min);
    void setMax(double max);

    void increase();
    void decrease();
    void changeBy(double v);

   signals:
    /**
     * @brief Number changed.
     */
    emits<double> valueChanged;

    /**
     * @brief Number is changing.
     */
    emits<> valueChanging;
};

namespace aui::impl {
template <typename Num>
struct ADataBindingDefaultDoubleNumberPicker {
   public:
    static void setup(const _<ADoubleNumberPicker>& view) {}

    static auto getGetter() { return &ADoubleNumberPicker::valueChanged; }

    static auto getSetter() { return &ADoubleNumberPicker::setValue; }
};

template <aui::arithmetic UnderlyingType, auto min, auto max>
    requires aui::convertible_to<decltype(min), UnderlyingType> && aui::convertible_to<decltype(max), UnderlyingType>
struct ADataBindingRangedDoubleNumberPicker {
   public:
    static void setup(const _<ADoubleNumberPicker>& view) {
        view->setMin(aui::ranged_number<UnderlyingType, min, max>::MIN);
        view->setMax(aui::ranged_number<UnderlyingType, min, max>::MAX);
    }

    static auto getGetter() { return &ADoubleNumberPicker::valueChanged; }

    static auto getSetter() { return &ADoubleNumberPicker::setValue; }
};
}   // namespace aui::impl

template <>
struct ADataBindingDefault<ADoubleNumberPicker, double> : aui::impl::ADataBindingDefaultDoubleNumberPicker<double> {};

template <>
struct ADataBindingDefault<ADoubleNumberPicker, float> : aui::impl::ADataBindingDefaultDoubleNumberPicker<float> {};

template <aui::arithmetic UnderlyingType, auto min, auto max>
    requires aui::convertible_to<decltype(min), UnderlyingType> && aui::convertible_to<decltype(max), UnderlyingType>
struct ADataBindingDefault<ADoubleNumberPicker, aui::ranged_number<UnderlyingType, min, max>>
    : aui::impl::ADataBindingRangedDoubleNumberPicker<UnderlyingType, min, max> {};
