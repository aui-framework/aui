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

#include "ANumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/Platform.h"
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
            if (mPicker.getValue() > mPicker.getMin()) {
                if (AInput::isKeyDown(AInput::LCONTROL)) {
                    mPicker.setValue(glm::max(mPicker.getValue() - 10, mPicker.getMin()));
                } else {
                    mPicker.setValue(mPicker.getValue() - 1);
                }
            }
            break;
        case AInput::UP:
            if (mPicker.getValue() < mPicker.getMax()) {
                if (AInput::isKeyDown(AInput::LCONTROL)) {
                    mPicker.setValue(glm::min(mPicker.getValue() + 10, mPicker.getMax()));
                } else {
                    mPicker.setValue(mPicker.getValue() + 1);
                }
            }
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

	connect(up->clicked, this, [&]()
	{
		setValue(getValue() + 1);
	});
	connect(down->clicked, this, [&]()
	{
		setValue(getValue() - 1);
	});

	connect(mTextField->textChanged, this, [&]()
	{
		int v = getValue();
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

int ANumberPicker::getContentMinimumHeight()
{
	return AViewContainer::getContentMinimumHeight();
}

void ANumberPicker::setValue(int v)
{
	mTextField->setText(AString::number(v));
	emit valueChanging();
    redraw();
}

int ANumberPicker::getValue() const
{
	return *mTextField->text().toInt();
}

void ANumberPicker::setMin(const int min)
{
	mMin = min;
	if (getValue() < min)
		setValue(min);
}

void ANumberPicker::setMax(const int max)
{
	mMax = max;
	if (getValue() > max)
		setValue(max);
}

