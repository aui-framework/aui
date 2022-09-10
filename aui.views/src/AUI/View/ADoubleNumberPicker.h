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
class API_AUI_VIEWS ADoubleNumberPicker: public AViewContainer
{
private:
    class ADoubleNumberPickerField: public AAbstractTextField
    {
    private:
        ADoubleNumberPicker& mPicker;
    public:
        ADoubleNumberPickerField(::ADoubleNumberPicker& picker)
                : mPicker(picker)
        {
        }

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

    void setValue(int v);
    int getValue() const;


    [[nodiscard]] int getMin() const
    {
        return mMin;
    }

    [[nodiscard]] int getMax() const
    {
        return mMax;
    }


    void setMin(double min);
    void setMax(double max);

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
    template<typename Num>
    struct ADataBindingDefaultNumberPicker {
    public:

        static void setup(const _<ADoubleNumberPicker>& view) {
        }

        static auto getGetter() {
            return &ADoubleNumberPicker::valueChanged;
        }

        static auto getSetter() {
            return &ADoubleNumberPicker::setValue;
        }
    };
}

template<> struct ADataBindingDefault<ADoubleNumberPicker, double>: aui::impl::ADataBindingDefaultNumberPicker<double> {};
template<> struct ADataBindingDefault<ADoubleNumberPicker, float>: aui::impl::ADataBindingDefaultNumberPicker<float> {};