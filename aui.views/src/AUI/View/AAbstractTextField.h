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

#include <AUI/View/AAbstractTypeableView.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Common/IStringable.h>
#include <AUI/Render/Render.h>

/**
 * @brief Text field implementation
 * @details ATextField is separated into the different class in order to simplify styling.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AAbstractTextField : public AAbstractTypeableView, public IStringable
{
private:
    bool mIsPasswordTextField = false;

	void invalidatePrerenderedString() override;
	AString getContentsPasswordWrap();

protected:
    Render::PrerenderedString mPrerenderedString;
    AString mContents;
	virtual bool isValidText(const AString& text);

    void prerenderStringIfNeeded();

    void typeableErase(size_t begin, size_t end) override;
    bool typeableInsert(size_t at, const AString& toInsert) override;
    size_t typeableFind(wchar_t c, size_t startPos) override;
    size_t typeableReverseFind(wchar_t c, size_t startPos) override;
    size_t length() const override;

    bool typeableInsert(size_t at, wchar_t toInsert) override;

    AString getDisplayText() override;

    void doRedraw() override;


    void doDrawString();

public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	int getContentMinimumHeight(ALayoutDirection layout) override;
	void setText(const AString& t) override;

    void render() override;

    AString toString() const override;

    void setPasswordMode(bool isPasswordMode) {
        mIsPasswordTextField = isPasswordMode;
    }

	bool isPasswordMode() const {
		return mIsPasswordTextField;
	}

	bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

    const AString& text() const override;

    size_t textLength() const override;

    void onCharEntered(wchar_t c) override;
    void invalidateFont() override;

};
