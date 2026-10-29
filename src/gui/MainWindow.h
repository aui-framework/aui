#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AAsyncHolder.h>

#include "remote/Discovery.h"

class ACheckBox;
class ADrawableView;
class ATimer;

/**
 * @brief The GUI of aui-remote: running applications, pairing, a look at the selected application and the switch that
 * installs the skill for AI agents.
 */
class MainWindow : public AWindow {
public:
    MainWindow();
    ~MainWindow() override;

private:
    AProperty<AVector<aui_remote::AppInfo>> mApps;
    AProperty<AOptional<std::uint32_t>> mSelectedPid;
    AProperty<AString> mStatus;
    AProperty<AString> mAppTitle;
    AProperty<AString> mTree;
    AProperty<bool> mBusy = false;
    _<AViewContainer> mAppList;
    _<AViewContainer> mAppActions;
    _<ADrawableView> mScreenshot;
    _<ACheckBox> mSkillCheckBox;
    _<ATimer> mTimer;
    AAsyncHolder mAsync;

    [[nodiscard]] AOptional<aui_remote::AppInfo> selectedApp() const;
    [[nodiscard]] bool isPaired(const aui_remote::AppInfo& app) const;

    void refreshApps();
    void rebuildAppList();
    void rebuildActions();
    void select(std::uint32_t pid);
    void pair();
    void inspect();
    void skillToggled(bool wanted);
    void syncSkillCheckBox();
};
