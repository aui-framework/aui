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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Thread/AAsyncHolder.h>
#include "AViewContainer.h"
#include "ATextField.h"

/**
 * @brief A text field with "..." button prompting path to a file.
 *
 * ![](imgs/Views/AFileChooserView.png)
 *
 * @ingroup views_input
 */
class API_AUI_VIEWS AFileChooserView: public AViewContainerBase {
private:
    _<ATextField> mPathField;
    AAsyncHolder mAsync;

public:
    explicit AFileChooserView(const APath& defaultPath = "", AVector<ADesktop::FileExtension> extensions = { {"All", "*"} });

    void setPath(const APath& path);
    APath getPath() const;

    auto path() {
        return APropertyDef {
            this,
            &AFileChooserView::getPath,
            &AFileChooserView::setPath,
            changed,
        };
    }

signals:
    emits<APath> changed;
};



template<>
struct ADataBindingDefault<AFileChooserView, APath> {
public:
    static auto setup(const _<AFileChooserView>& v) {}

    static auto property(const _<AFileChooserView>& v) {
        return v->path();
    }

    static auto getGetter() {
        return &AFileChooserView::changed;
    }
    static auto getSetter() {
        return &AFileChooserView::setPath;
    }
};

/**
 * @brief A text field with "..." button prompting path to a dir.
 *
 * ![](imgs/Views/ADirChooserView.png)
 *
 * @ingroup views_input
 */
class API_AUI_VIEWS ADirChooserView: public AViewContainerBase {
private:
    _<ATextField> mPathField;
    AAsyncHolder mAsync;

public:
    explicit ADirChooserView(const APath& defaultPath = "");

    void setPath(const APath& path);
    APath getPath() const;

    auto path() {
        return APropertyDef {
            this,
            &ADirChooserView::getPath,
            &ADirChooserView::setPath,
            changed,
        };
    }

signals:
    emits<APath> changed;
};



template<>
struct ADataBindingDefault<ADirChooserView, APath> {
public:
    static auto setup(const _<ADirChooserView>& v) {}

    static auto property(const _<ADirChooserView>& v) {
        return v->path();
    }

    static auto getGetter() {
        return &ADirChooserView::changed;
    }
    static auto getSetter() {
        return &ADirChooserView::setPath;
    }
};

using APathChooserView [[deprecated("APathChooserView was renamed to ADirChooserView")]] = ADirChooserView;
