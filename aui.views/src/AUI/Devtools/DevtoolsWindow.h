//#ifdef _DEBUG
#pragma once

#include <AUI/Platform/AWindow.h>
#include "AUI/View/ATreeView.h"
#include "ViewPropertiesView.h"

class DevtoolsWindow: public AWindow {
private:
    ABaseWindow* mTargetWindow;

    _<ATreeView> mViewHierarchyTree;
    _<ViewPropertiesView> mViewPropertiesView;

public:
    DevtoolsWindow(ABaseWindow* targetWindow);

};

//#endif