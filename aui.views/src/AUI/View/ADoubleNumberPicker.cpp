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

#include "ADoubleNumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ADesktop.h"
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
    setLayout(std::make_unique<AHorizontalLayout>());
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
    auto up = _new<AButton>() AUI_LET { it->setDefault(); it  << ".up"; };
    auto down = _new<AButton>() AUI_LET { it->setDefault(); it  << ".down"; };
    c->setLayout(std::make_unique<AVerticalLayout>());
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
            ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
        } else if (v > mMax)
        {
            v = mMax;
            ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
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

    connect(mTextField->textChanging, this, [this]()
    {
        emit valueChanging(getValue());
    });

    addView(c);
}

void ADoubleNumberPicker::setValue(double v)
{
    mTextField->setText(AString::number(v));
    redraw();
}

void ADoubleNumberPicker::setSuffix(const AString& suffix)
{
    mTextField->setSuffix(suffix);
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
    v += getValue();
    setValue(v);
    emit valueChanging(v);
}
