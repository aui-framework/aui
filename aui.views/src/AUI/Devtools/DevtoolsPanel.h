//#ifdef _DEBUG
#pragma once

#include "AUI/View/ATreeView.h"
#include "ViewPropertiesView.h"

class DevtoolsPanel: public AViewContainer {
private:
    AViewContainer* mTargetWindow;

    _<ATreeView> mViewHierarchyTree;
    _<ViewPropertiesView> mViewPropertiesView;

public:
    DevtoolsPanel(AViewContainer* targetWindow);

};

//#endif