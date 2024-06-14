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
#include "ATextField.h"
#include <limits>

/**
 * @brief A text field for numbers with increase/decrease buttons.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ANumberPicker: public AViewContainer
{
private:
    class ANumberPickerField: public AAbstractTextField
    {
    private:
        ANumberPicker& mPicker;
    public:
        ANumberPickerField(::ANumberPicker& picker)
                : mPicker(picker)
        {
        }

        virtual ~ANumberPickerField() = default;

        void onKeyRepeat(AInput::Key key) override;

    protected:
        bool isValidText(const AString& text) override;
    };

    _<ANumberPickerField> mTextField;

    int64_t mMin = 0;
    int64_t mMax = 100;

public:
    ANumberPicker();

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void setValue(int64_t v);
    int64_t getValue() const;

    [[nodiscard]]
    const AString& text() const noexcept {
        return mTextField->text();
    }

    void increase();
    void decrease();
    void changeBy(int64_t v);


    [[nodiscard]] int64_t getMin() const
    {
        return mMin;
    }

    [[nodiscard]] int64_t getMax() const
    {
        return mMax;
    }


    void setMin(int64_t min);
    void setMax(int64_t max);

signals:
    /**
     * @brief Number changed.
     */
    emits<int64_t> valueChanged;

    /**
     * @brief Number is changing.
     */
    emits<> valueChanging;
};

namespace aui::impl {
    template<typename Num>
    struct ADataBindingDefaultNumberPicker {
    public:

        static void setup(const _<ANumberPicker>& view) {
            view->setMin((std::numeric_limits<Num>::min)());
            view->setMax((std::numeric_limits<Num>::max)());
        }

        static auto getGetter() {
            return &ANumberPicker::valueChanged;
        }

        static auto getSetter() {
            return &ANumberPicker::setValue;
        }
    };
}

template<> struct ADataBindingDefault<ANumberPicker, uint8_t>: aui::impl::ADataBindingDefaultNumberPicker<uint8_t> {};
template<> struct ADataBindingDefault<ANumberPicker, int8_t>: aui::impl::ADataBindingDefaultNumberPicker<int8_t> {};
template<> struct ADataBindingDefault<ANumberPicker, uint16_t>: aui::impl::ADataBindingDefaultNumberPicker<uint16_t> {};
template<> struct ADataBindingDefault<ANumberPicker, int16_t>: aui::impl::ADataBindingDefaultNumberPicker<int16_t> {};
template<> struct ADataBindingDefault<ANumberPicker, uint32_t>: aui::impl::ADataBindingDefaultNumberPicker<uint32_t> {};
template<> struct ADataBindingDefault<ANumberPicker, int32_t>: aui::impl::ADataBindingDefaultNumberPicker<int32_t> {};
template<> struct ADataBindingDefault<ANumberPicker, int64_t>: aui::impl::ADataBindingDefaultNumberPicker<int64_t> {};
