// AUI Framework - Declarative UI toolkit for modern C++20
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