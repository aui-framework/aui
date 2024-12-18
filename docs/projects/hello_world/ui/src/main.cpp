/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2022 Alex2772
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Util/ADataBinding.h>

struct MyModel {
    AString data;
};

class MyWindow: public AWindow {
private:
    _<ADataBinding<MyModel>> mBinding = _new<ADataBinding<MyModel>>();
    const MyModel mModel;

public:

    MyWindow(const MyModel& model):
        AWindow("Hello world", 300_dp, 200_dp),
        mModel(model)
    {
        mBinding->setModel(mModel);

        setContents(
            Stacked {
                _new<ALabel>("Hello world!") && mBinding(&MyModel::data, &ALabel::setText)
            }
        );
    }
};

AUI_ENTRY {
    _new<MyWindow>({"DATA!!!!"})->show();

    return 0;
}