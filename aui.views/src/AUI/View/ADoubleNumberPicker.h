// AUI Framework - Declarative UI toolkit for modern C++17
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

    void setValue(double v);

    [[nodiscard]]
    const AString& text() const noexcept {
        return mTextField->text();
    }

    [[nodiscard]]
    double getValue() const {
        return mTextField->text().toDouble().valueOr(0.0);
    }


    [[nodiscard]] double getMin() const
    {
        return mMin;
    }

    [[nodiscard]] double getMax() const
    {
        return mMax;
    }


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
    template<typename Num>
    struct ADataBindingDefaultDoubleNumberPicker {
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

template<> struct ADataBindingDefault<ADoubleNumberPicker, double>: aui::impl::ADataBindingDefaultDoubleNumberPicker<double> {};
template<> struct ADataBindingDefault<ADoubleNumberPicker, float>: aui::impl::ADataBindingDefaultDoubleNumberPicker<float> {};