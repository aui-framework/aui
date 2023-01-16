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

//
// Created by alex2 on 24.10.2020.
//

#pragma once

#include "APageView.h"
#include "ALabel.h"

class API_AUI_VIEWS ATabButtonView: public ALabel, public ass::ISelectable {
private:
    bool mCurrent = false;
protected:
    bool selectableIsSelectedImpl() override;

public:
    ATabButtonView() {

    }
    ATabButtonView(const AString& text): ALabel(text) {

    }

    void setCurrent(bool current);
};

class API_AUI_VIEWS ATabButtonRow: public AViewContainer {
private:
    _<AViewContainer> mContents;

public:
    ATabButtonRow();
    void setCurrent(size_t i, bool current);
    void addTab(const AString& name);

    const _<AViewContainer>& getContents() const {
        return mContents;
    }
};

/**
 * @brief Tab host
 * @ingroup useful_views
 * @details
 * Used for screen space saving by grouping views.
 *
 * Unlike APageView, has a row with buttons to switch tabs (pages).
 */
class API_AUI_VIEWS ATabView: public AViewContainer {
private:
    _<APageView> mPageView;
    _<ATabButtonRow> mRow;

public:
    ATabView();

    void addTab(const _<AView>& view, const AString& name = {});
    void setTabId(unsigned tabId);

};


