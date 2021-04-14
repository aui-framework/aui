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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AUI/View/AView.h"
#include "AUI/Common/SharedPtr.h"

class AViewContainer;

class API_AUI_VIEWS ALayout: public AObject
{
protected:
	ADeque<_<AView>> mViews;
	
public:
	ALayout() = default;
	virtual ~ALayout() = default;
	virtual void onResize(int x, int y, int width, int height) = 0;
	virtual void addView(size_t index, const _<AView>& view);
	virtual void removeView(size_t index, const _<AView>& view);
	virtual int getMinimumWidth() = 0;
	virtual int getMinimumHeight() = 0;
	virtual void setSpacing(int spacing);
};
