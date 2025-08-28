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


#include "AViewContainer.h"

/**
 * @brief Like ATabView but without tabs line.
 * @ingroup views_actions
 */
class API_AUI_VIEWS APageView: public AViewContainerBase {
private:
    unsigned mPageId = 0;

public:
    APageView();

    void addPage(const _<AView>& view);
    void setPageId(unsigned int pageId);

    unsigned getPageId() const {
        return mPageId;
    }

signals:
    emits<unsigned> pageChanging;
    emits<> pageChanged;

};


