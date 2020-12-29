#include "ANumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Image/Drawables.h"
#include "AUI/Platform/Platform.h"

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
        case AInput::Down:
            if (mPicker.getValue() > mPicker.getMin()) {
                if (AInput::isKeyDown(AInput::LControl)) {
                    mPicker.setValue(glm::max(mPicker.getValue() - 10, mPicker.getMin()));
                } else {
                    mPicker.setValue(mPicker.getValue() - 1);
                }
            }
            break;
        case AInput::Up:
            if (mPicker.getValue() < mPicker.getMax()) {
                if (AInput::isKeyDown(AInput::LControl)) {
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
	AVIEW_CSS;
	
	setLayout(_new<AHorizontalLayout>());
	addView(mTextField = _new<ANumberPickerField>(*this));
	addCssName(".input-field");

	mTextField->setExpanding({ 1, 1 });
	connect(mTextField->focusState, this, [&](bool c)
	{
		if (c)
			onFocusAcquired();
		else
			onFocusLost();
	});
	
	auto c = _new<AViewContainer>();
	c->addCssName(".up-down-wrapper");
	auto up = _new<AButton>();
	up->setIcon(Drawables::get(":win/svg/up.svg"));
	auto down = _new<AButton>();
	down->setIcon(Drawables::get(":win/svg/down.svg"));
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
}

int ANumberPicker::getValue() const
{
	return mTextField->getText().toInt();
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

