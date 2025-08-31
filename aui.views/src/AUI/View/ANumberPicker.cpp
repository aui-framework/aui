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

#include "ANumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/ADesktop.h"
#include <AUI/Util/kAUI.h>

bool ANumberPicker::ANumberPickerField::isValidText(const AString& text) {
    if (text.length() > 10)
        return false;
    for (auto c : text) {
        if ((c < '0' || c > '9') && c != '-')
            return false;
    }

    return true;
}

void ANumberPicker::ANumberPickerField::onKeyRepeat(AInput::Key key) {

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


ANumberPicker::ANumberPicker() {


    setLayout(std::make_unique<AHorizontalLayout>());
    addView(mTextField = _new<ANumberPickerField>(*this));
    addAssName(".input-field");
    addAssName(".number-picker");

    mTextField->setExpanding({1, 1});
    connect(mTextField->focusState, this, [&](bool c) {
        if (c)
            onFocusAcquired();
        else
            onFocusLost();
    });

    auto c = _new<AViewContainer>();
    c->addAssName(".up-down-wrapper");
    auto up = _new<AButton>() AUI_LET {
        it->setDefault();
        it << ".up";
    };
    auto down = _new<AButton>() AUI_LET {
        it->setDefault();
        it << ".down";
    };
    c->setLayout(std::make_unique<AVerticalLayout>());
    c->setExpanding({0, 0});
    c->addView(up);
    c->addView(down);

    connect(up->clicked, me::increase);
    connect(down->clicked, me::decrease);

    connect(mTextField->textChanged, this, [&]() {
        int64_t v = getValue();
        if (v < mMin) {
            v = mMin;
            ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
        } else if (v > mMax) {
            v = mMax;
            ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
        } else {
            emit valueChanged(v);
            return;
        }
        mTextField->setSignalsEnabled(false);
        setValue(v);
        mTextField->setSignalsEnabled(true);
        emit valueChanged(v);
    });

    connect(mTextField->textChanging, this, [&]() {
        emit valueChanging(getValue());
    });

    addView(c);
}

void ANumberPicker::setValue(int64_t v) {
    mTextField->setText(AString::number(v));
    redraw();
}

int64_t ANumberPicker::getValue() const {
    return mTextField->getText().toInt().valueOr(0);
}

void ANumberPicker::setSuffix(const AString& suffix) {
    mTextField->setSuffix(suffix);
    redraw();
}

void ANumberPicker::setMin(int64_t min) {
    mMin = min;
    if (getValue() < min)
        setValue(min);
}

void ANumberPicker::setMax(int64_t max) {
    mMax = max;
    if (getValue() > max)
        setValue(max);
}


void ANumberPicker::increase() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? 10 : 1);
}

void ANumberPicker::decrease() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? -10 : -1);
}

void ANumberPicker::changeBy(int64_t v) {
    setValue(getValue() + v);
    emit valueChanging(getValue());
}
