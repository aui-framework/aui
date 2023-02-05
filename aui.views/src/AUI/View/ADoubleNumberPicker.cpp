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

#include "ADoubleNumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/Platform.h"
#include <AUI/Util/kAUI.h>

bool ADoubleNumberPicker::ADoubleNumberPickerField::isValidText(const AString& text)
{
    return text.toDouble().hasValue() || text.empty();
}

void ADoubleNumberPicker::ADoubleNumberPickerField::onKeyRepeat(AInput::Key key) {
    switch (key) {
        case AInput::DOWN:
            mPicker.decrease();
            break;
        case AInput::UP:
            mPicker.increase();
            break;
        default:
            AAbstractTextField::onKeyRepeat(key);
    }
}


ADoubleNumberPicker::ADoubleNumberPicker()
{
    setLayout(_new<AHorizontalLayout>());
    addView(mTextField = _new<ADoubleNumberPickerField>(*this));
    addAssName(".input-field");
    addAssName(".number-picker");

    mTextField->setExpanding({ 1, 1 });
    connect(mTextField->focusState, this, [&](bool c)
    {
        if (c)
            onFocusAcquired();
        else
            onFocusLost();
    });

    auto c = _new<AViewContainer>();
    c->addAssName(".up-down-wrapper");
    auto up = _new<AButton>() let { it->setDefault(); it  << ".up"; };
    auto down = _new<AButton>() let { it->setDefault(); it  << ".down"; };
    c->setLayout(_new<AVerticalLayout>());
    c->setExpanding({ 0, 0 });
    c->addView(up);
    c->addView(down);

    connect(up->clicked, me::increase);
    connect(down->clicked, me::decrease);

    connect(mTextField->textChanged, this, [&]()
    {
        double v = getValue();
        if (v < mMin)
        {
            v = mMin;
            Platform::playSystemSound(Platform::S_ASTERISK);
        } else if (v > mMax)
        {
            v = mMax;
            Platform::playSystemSound(Platform::S_ASTERISK);
        } else
        {
            emit valueChanged(v);
            return;
        }
        mTextField->setSignalsEnabled(false);
        setValue(v);
        mTextField->setSignalsEnabled(true);
        emit valueChanged(v);
    });

    connect(mTextField->textChanging, this, [&]()
    {
        emit valueChanging();
    });

    addView(c);
}

int ADoubleNumberPicker::getContentMinimumHeight(ALayoutDirection layout)
{
    return AViewContainer::getContentMinimumHeight(ALayoutDirection::NONE);
}

void ADoubleNumberPicker::setValue(double v)
{
    mTextField->setText(AString::number(v));
    redraw();
}


void ADoubleNumberPicker::setMin(double min)
{
    mMin = min;
    if (getValue() < min)
        setValue(min);
}

void ADoubleNumberPicker::setMax(double max)
{
    mMax = max;
    if (getValue() > max)
        setValue(max);
}


void ADoubleNumberPicker::increase() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? 0.1 : 1);
}

void ADoubleNumberPicker::decrease() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? -0.1 : -1);
}

void ADoubleNumberPicker::changeBy(double v) {
    setValue(getValue() + v);
    emit valueChanging();
}
