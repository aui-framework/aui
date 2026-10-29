#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AAsyncHolder.h>

#include "network/Discovery.h"

class AppTab;
class ATimer;
class WelcomePage;

/**
 * @brief The GUI of aui-remote: a strip of tabs on top. The first tab is the Welcome page, the others are connections.
 */
class MainWindow : public AWindow {
public:
    MainWindow();
    ~MainWindow() override;

private:
    _<WelcomePage> mWelcome;
    AProperty<AVector<_<AppTab>>> mTabs;
    AVector<aui_remote::AppInfo> mApps;
    AProperty<AOptional<std::uint32_t>> mCurrentPid;   // nullopt = Welcome
    _<ATimer> mTimer;
    AAsyncHolder mAsync;

    [[nodiscard]] _<AppTab> findTab(std::uint32_t pid) const;

    void refreshApps();
    void updateWelcome();
    void pair(const aui_remote::AppInfo& app);
    void openTab(const aui_remote::AppInfo& app);
    void closeTab(std::uint32_t pid);
    [[nodiscard]] _<AView> tabStrip();
    [[nodiscard]] _<AView> currentPage();
};
