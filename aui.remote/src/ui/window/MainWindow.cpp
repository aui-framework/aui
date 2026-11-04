#include "MainWindow.h"

#include <AUI/Common/AProperty.h>
#include <AUI/Common/ATimer.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ATitlebar.h>
#include <AUI/View/Dynamic.h>

#include "PairingDialog.h"
#include "ui/tab/AppTab.h"
#include "ui/tab/WelcomePage.h"
#include "network/KeyStore.h"

using namespace declarative;
using namespace aui_remote;
using namespace std::chrono_literals;

namespace {

AColor dotColor(AppTabModel::State state) {
    switch (state) {
        case AppTabModel::State::READY: return AColor::GREEN.darker(0.3f);
        case AppTabModel::State::CONNECTING: return 0xffe0a000_argb;
        case AppTabModel::State::NEEDS_PAIRING: return 0xffe0a000_argb;
        case AppTabModel::State::FAILED: return AColor::RED;
        case AppTabModel::State::EXITED: return 0xff999999_argb;
    }
    return 0xff999999_argb;
}

struct Tab {
    _<AppTabModel> model;
    _<AView> view;
};

struct State : AObject {
    _<AProperty<AVector<ConnectionRow>>> rows = _new<AProperty<AVector<ConnectionRow>>>();
    _<AView> welcome;
    AProperty<AVector<Tab>> tabs;
    AVector<AppInfo> apps;
    AProperty<AOptional<std::uint32_t>> currentPid;   // nullopt = Welcome
    _<ATimer> timer;
    AAsyncHolder async;

    [[nodiscard]] AOptional<Tab> findTab(std::uint32_t pid) const {
        for (const auto& tab : *tabs) {
            if (tab.model->app.pid == pid) {
                return tab;
            }
        }
        return std::nullopt;
    }

    void refreshApps() {
        async << AUI_THREADPOOL {
            auto found = discovery::list();
            KeyStore().pruneExcept(found);
            // a debug build is an application with a remote agent too; it is not shown to itself
            found.removeIf([self = AProcess::self()->getPid()](const AppInfo& app) { return app.pid == self; });
            AUI_UI_THREAD {
                apps = found;
                updateWelcome();
                for (const auto& tab : *tabs) {
                    tab.model->alive = found.contains(tab.model->app);
                }
            };
        };
    }

    void updateWelcome() {
        AVector<ConnectionRow> result;
        KeyStore store;
        for (const auto& app : apps) {
            auto action = store.load(app).hasValue() ? ConnectionRow::Action::CONNECT : ConnectionRow::Action::PAIR;
            if (findTab(app.pid)) {
                action = ConnectionRow::Action::OPEN;
            }
            result << ConnectionRow { app, action };
        }
        if (**rows != result) {
            *rows = std::move(result);
        }
    }

    void pair(const AppInfo& app, std::function<void()> onPaired) {
        auto* parent = dynamic_cast<AWindow*>(AWindow::current());
        if (!parent) {
            return;
        }
        auto dialog = _new<AWindow>(AWindow::Init {
          .title = "Pair with " + app.displayName(),
          .width = 380_dp,
          .height = 170_dp,
          .parent = AWindow::Init::Parent { .window = *parent, .modal = true },
          .resizable = false,
        });
        dialog->setContents(pairingDialog(app, std::move(onPaired)));
        dialog->show();
    }

    void openTab(const AppInfo& app) {
        if (!findTab(app.pid)) {
            auto model = aui::ptr::manage_shared(new AppTabModel { .app = app });
            tabs.writeScope()->push_back(Tab {
              model,
              appTab(model, [this](const AppInfo& target, std::function<void()> onPaired) {
                  pair(target, std::move(onPaired));
              }),
            });
            updateWelcome();
        }
        currentPid = app.pid;
    }

    void closeTab(std::uint32_t pid) {
        tabs.writeScope()->removeIf([&](const Tab& tab) { return tab.model->app.pid == pid; });
        updateWelcome();
        if (*currentPid == pid) {
            currentPid = std::nullopt;
        }
    }

    [[nodiscard]] _<AView> tabStrip() {
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
            tabItem(Label { "Welcome" }, !currentPid->hasValue(), [this] { currentPid = std::nullopt; }),
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Padding { 4_dp, 4_dp, 0, 4_dp } };

        for (const auto& tab : *tabs) {
            const auto pid = tab.model->app.pid;
            auto close = Label { "×" } AUI_OVERRIDE_STYLE { Padding { 0, 4_dp } };
            connect(close->clicked, this, [this, pid] { closeTab(pid); });
            strip->addView(tabItem(
                Horizontal {
                  Label { "●" } AUI_OVERRIDE_STYLE { TextColor { dotColor(*tab.model->state) } },
                  Label { tab.model->title() },
                  close,
                } AUI_OVERRIDE_STYLE { LayoutSpacing { 6_dp } },
                *currentPid == pid, [this, pid] { currentPid = pid; }));
        }
        return strip;
    }

    [[nodiscard]] _<AView> currentPage() {
        auto page = welcome;
        if (currentPid->hasValue()) {
            if (auto tab = findTab(**currentPid)) {
                page = tab->view;
            }
        }
        page->setExpanding();
        return page;
    }
};

}   // namespace

_<AViewContainer> mainWindowContents() {
    auto state = _new<State>();
    state->welcome = welcomePage(
        state->rows,
        [state = state.get()](AppInfo app) {
            state->pair(app, [weak = _weak<State>(aui::ptr::shared_from_this(state)), app] {
                if (auto s = weak.lock()) {
                    s->openTab(app);
                }
            });
        },
        [state = state.get()](AppInfo app) { state->openTab(app); });

    return Vertical {
      Titlebar {
        experimental::Dynamic { AUI_REACT(state->tabStrip()) },   // owns the state
      } AUI_OVERRIDE_STYLE { BackgroundSolid { nullptr } },
      experimental::Dynamic { AUI_REACT(state->currentPage()) } AUI_OVERRIDE_STYLE { Expanding() },
    } AUI_LET {
        state->timer = _new<ATimer>(2s);
        AObject::connect(state->timer->fired, state, [state = state.get()] { state->refreshApps(); });
        state->timer->start();
        state->refreshApps();
    };
}
