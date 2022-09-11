/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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

    int mMin = 0;
    int mMax = 100;

public:
    ANumberPicker();

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void setValue(int v);
    int getValue() const;

    void increase();
    void decrease();
    void changeBy(int v);


    [[nodiscard]] int getMin() const
    {
        return mMin;
    }

    [[nodiscard]] int getMax() const
    {
        return mMax;
    }


    void setMin(const int min);
    void setMax(const int max);

signals:
    /**
     * @brief Number changed.
     */
    emits<int> valueChanged;

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
            view->setMin((std::numeric_limits<int>::min)());
            view->setMax((std::numeric_limits<int>::max)());
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

