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

class API_AUI_VIEWS ATabButtonRow: public AViewContainerBase {
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
 *
 * ![](imgs/Views/ATabView.png)
 *
 * @ingroup views_actions
 * @details
 * Used for screen space saving by grouping views.
 *
 * Unlike APageView, has a row with buttons to switch tabs (pages).
 */
class API_AUI_VIEWS ATabView: public AViewContainerBase {
private:
    _<APageView> mPageView;
    _<ATabButtonRow> mRow;

public:
    ATabView();

    void addTab(const _<AView>& view, const AString& name = {});
    void setTabId(unsigned tabId);

};


