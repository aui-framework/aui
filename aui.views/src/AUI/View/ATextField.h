// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AView.h"
#include "AAbstractTextField.h"
#include <AUI/Util/ADataBinding.h>

/**
 * @brief Represents an editable field with text to receive a text input from the user.
 * @ingroup useful_views
 * @details
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ATextField.gif">
 */
class API_AUI_VIEWS ATextField: public AAbstractTextField
{
public:
	ATextField();
	~ATextField() override;

protected:
	bool isValidText(const AString& text) override;
};

template<>
struct ADataBindingDefault<ATextField, AString> {
public:
    static void setup(const _<ATextField>& view) {}
    static auto getGetter() {
        return (ASignal<AString> ATextField::*)&ATextField::textChanging;
    }
    static auto getSetter() {
        return (void(ATextField::*)(const AString& t)) &ATextField::setText;
    }
};
