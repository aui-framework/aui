#include "MainWindow.h"

#include <AUI/Common/ATimer.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ATitlebar.h>
#include <AUI/View/Dynamic.h>

#include "AppTab.h"
#include "PairingDialog.h"
#include "WelcomePage.h"
#include "remote/KeyStore.h"

using namespace declarative;
using namespace aui_remote;
using namespace std::chrono_literals;

namespace {

AColor dotColor(AppTab::State state) {
    switch (state) {
        case AppTab::State::READY: return AColor::GREEN.darker(0.3f);
        case AppTab::State::CONNECTING: return 0xffe0a000_argb;
        case AppTab::State::NEEDS_PAIRING: return 0xffe0a000_argb;
        case AppTab::State::ERROR: return AColor::RED;
        case AppTab::State::EXITED: return 0xff999999_argb;
    }
    return 0xff999999_argb;
}

}   // namespace

MainWindow::MainWindow()
  : AWindow(Init {
      .title = "AUI Remote",
      .width = 900_dp,
      .height = 600_dp,
      .decoration = Init::DecorationExtended {},
    }) {
    mWelcome = _new<WelcomePage>();

    setContents(Vertical {
      Titlebar {
        experimental::Dynamic { AUI_REACT(tabStrip()) },
      } AUI_OVERRIDE_STYLE { BackgroundSolid { nullptr } },
      experimental::Dynamic { AUI_REACT(currentPage()) } AUI_OVERRIDE_STYLE { Expanding() },
    });

    connect(mWelcome->openRequested, this, [this](const AppInfo& app) { openTab(app); });
    connect(mWelcome->pairRequested, this, [this](const AppInfo& app) { pair(app); });

    mTimer = _new<ATimer>(2s);
    connect(mTimer->fired, this, [this] { refreshApps(); });
    mTimer->start();
    refreshApps();
}

MainWindow::~MainWindow() = default;

_<AppTab> MainWindow::findTab(std::uint32_t pid) const {
    for (const auto& tab : *mTabs) {
        if (tab->app().pid == pid) {
            return tab;
        }
    }
    return nullptr;
}

void MainWindow::refreshApps() {
    mAsync << AUI_THREADPOOL {
        auto apps = discovery::list();
        KeyStore().pruneExcept(apps);
        // a debug build is an application with a remote agent too; it is not shown to itself
        apps.removeIf([self = AProcess::self()->getPid()](const AppInfo& app) { return app.pid == self; });
        AUI_UI_THREAD {
            mApps = apps;
            updateWelcome();
            for (const auto& tab : *mTabs) {
                bool alive = false;
                for (const auto& app : apps) {
                    if (app.pid == tab->app().pid) {
                        alive = true;
                        break;
                    }
                }
                tab->setAlive(alive);
            }
        };
    };
}

void MainWindow::updateWelcome() {
    AVector<std::uint32_t> openPids;
    for (const auto& tab : *mTabs) {
        openPids << tab->app().pid;
    }
    mWelcome->setApps(mApps, openPids);
}

void MainWindow::pair(const AppInfo& app) {
    // the tab appears only after the code is accepted
    auto dialog = _new<PairingDialog>(*this, app);
    connect(dialog->paired, this, [this, app] { openTab(app); });
    dialog->show();
}

void MainWindow::openTab(const AppInfo& app) {
    if (!findTab(app.pid)) {
        mTabs.writeScope()->push_back(_new<AppTab>(app));
        updateWelcome();
    }
    mCurrentPid = app.pid;
}

void MainWindow::closeTab(std::uint32_t pid) {
    mTabs.writeScope()->removeIf([&](const _<AppTab>& tab) { return tab->app().pid == pid; });
    updateWelcome();
    if (*mCurrentPid == pid) {
        mCurrentPid = std::nullopt;
    }
}

_<AView> MainWindow::tabStrip() {
    auto tabItem = [this](_<AView> content, bool selected, std::function<void()> onClick) {
        auto tab = Horizontal { std::move(content) } AUI_OVERRIDE_STYLE {
            LayoutSpacing { 4_dp },
            Padding { 4_dp, 8_dp },
            BorderRadius { 6_dp },
            BackgroundSolid { selected ? AColor::BLUE.transparentize(0.75f) : AColor(0x14000000_argb) },
        };
        tab->addAssName(".override-title-dragging");
        connect(tab->clicked, this, std::move(onClick));
        return tab;
    };

    auto strip = Horizontal {
        tabItem(Label { "Welcome" }, !mCurrentPid->hasValue(), [this] { mCurrentPid = std::nullopt; }),
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Padding { 4_dp, 4_dp, 0, 4_dp } };

    for (const auto& tab : *mTabs) {
        const auto pid = tab->app().pid;
        auto close = Label { "×" } AUI_OVERRIDE_STYLE { Padding { 0, 4_dp } };
        connect(close->clicked, this, [this, pid] { closeTab(pid); });
        strip->addView(tabItem(
            Horizontal {
              Label { "●" } AUI_OVERRIDE_STYLE { TextColor { dotColor(*tab->state()) } },
              Label { tab->title() },
              close,
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 6_dp } },
            *mCurrentPid == pid, [this, pid] { mCurrentPid = pid; }));
    }
    return strip;
}

_<AView> MainWindow::currentPage() {
    _<AViewContainer> page = mWelcome;
    if (mCurrentPid->hasValue()) {
        if (auto tab = findTab(**mCurrentPid)) {
            page = tab;
        }
    }
    page->setExpanding();
    return page;
}
