//
// Created by Alex2772 on 9/15/2022.
//

#include "ADragNDropView.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/Platform/ADragNDrop.h"

using namespace declarative;

ADragNDropView::ADragNDropView() {
    setContents(Centered {
        Label { "d&d test" },
    });
}

void ADragNDropView::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);

    if (AInput::isKeyDown(AInput::LBUTTON)) {
        if (!mDnDInProgress) {
            mDnDInProgress = true;
            ADragNDrop v;
            AMimedData data;
            data.setText("hello world!");
            v.setData(std::move(data));
            v.perform(getWindow());
        }
    } else {
        mDnDInProgress = false;
    }
}
