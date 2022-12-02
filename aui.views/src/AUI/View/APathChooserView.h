// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 15.04.2021.
//


#pragma once


#include "AViewContainer.h"
#include "ATextField.h"

/**
 * @brief A text field with "..." button prompting path.
 * @ingroup useful_views
 */
class API_AUI_VIEWS APathChooserView: public AViewContainer {
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