/**
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
#include "ALabel.h"
#include "AViewContainer.h"
#include <AUI/Util/ADataBinding.h>


class ACheckBoxInner: public AView
{
public:
    ACheckBoxInner() = default;
    virtual ~ACheckBoxInner() = default;

    void update();
};

/**
 * Represents a simple check box.
 */
class API_AUI_VIEWS ACheckBox: public AViewContainer
{
private:
	_<ALabel> mText;
	bool mChecked = false;
	
public:
	ACheckBox();
	ACheckBox(const ::AString& text);
	virtual ~ACheckBox();

	void setText(const AString& text);


	[[nodiscard]] bool isChecked() const
	{
		return mChecked;
	}

	void setChecked(bool checked = true)
    {
		mChecked = checked;
		emit customCssPropertyChanged();
		emit ACheckBox::checked(checked);
	}
	void setUnchecked(bool unchecked = true) {
	    setChecked(!unchecked);
	}

    bool consumesClick(const glm::ivec2& pos) override;

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
signals:
	emits<bool> checked;
};


template<>
struct ADataBindingDefault<ACheckBox, bool> {
public:
    static void setup(const _<ACheckBox>& view) {}
    static auto getGetter() {
        return &ACheckBox::checked;
    }
    static auto getSetter() {
        return &ACheckBox::setChecked;
    }
};
