//#ifdef _DEBUG
#pragma once

#include "AUI/View/ATreeView.h"
#include "AUI/Platform/ABaseWindow.h"
#include "ViewPropertiesView.h"

class DevtoolsPanel: public AViewContainer {
private:
    ABaseWindow* mTargetWindow;

    _<ATreeView> mViewHierarchyTree;
    _<ViewPropertiesView> mViewPropertiesView;

public:
    DevtoolsPanel(ABaseWindow* targetWindow);

};

//#endif