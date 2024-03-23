﻿// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "ANumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/ADesktop.h"
#include <AUI/Util/kAUI.h>

bool ANumberPicker::ANumberPickerField::isValidText(const AString& text)
{
	if (text.length() > 10)
		return false;
	for (auto& c : text)
	{
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


ANumberPicker::ANumberPicker()
{

	
	setLayout(_new<AHorizontalLayout>());
	addView(mTextField = _new<ANumberPickerField>(*this));
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
		int64_t v = getValue();
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

	connect(mTextField->textChanging, this, [&]()
	{
		emit valueChanging();
	});
	
	addView(c);
}

int ANumberPicker::getContentMinimumHeight(ALayoutDirection layout)
{
	return AViewContainer::getContentMinimumHeight(ALayoutDirection::NONE);
}

void ANumberPicker::setValue(int64_t v)
{
	mTextField->setText(AString::number(v));
    redraw();
}

int64_t ANumberPicker::getValue() const
{
	return mTextField->text().toInt().valueOr(0);
}

void ANumberPicker::setMin(int64_t min)
{
	mMin = min;
	if (getValue() < min)
		setValue(min);
}

void ANumberPicker::setMax(int64_t max)
{
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
    emit valueChanging();
}
