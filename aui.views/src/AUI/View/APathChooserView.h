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
// Created by alex2 on 15.04.2021.
//


#pragma once


#include "AViewContainer.h"
#include "ATextField.h"

/**
 * @brief A text field with "..." button prompting path.
 * @ingroup useful_views
 */
class API_AUI_VIEWS APathChooserView: public AViewContainerBase {
private:
    _<ATextField> mPathField;

public:
    explicit APathChooserView(const APath& defaultPath = "");

    void setPath(const APath& path);
    APath getPath() const;

signals:
    emits<APath> changed;
};



template<>
struct ADataBindingDefault<APathChooserView, APath> {
public:
    static auto setup(const _<APathChooserView>& v) {}

    static auto getGetter() {
        return &APathChooserView::changed;
    }
    static auto getSetter() {
        return &APathChooserView::setPath;
    }
};