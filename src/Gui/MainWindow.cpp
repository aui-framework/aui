#include "MainWindow.h"

#include <AUI/Common/ATimer.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/AImageDrawable.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>

#include "PairingDialog.h"
#include "Remote/KeyStore.h"
#include "Remote/RpcClient.h"
#include "Remote/Skill.h"

using namespace declarative;
using namespace aui_remote;
using namespace std::chrono_literals;

namespace {

void renderTree(const AJson& node, int depth, AString& out) {
    out += AString(depth * 2, ' ');
    out += node.containsOpt("type") ? (node)["type"].asString() : AString("?");
    if (auto text = node.containsOpt("text"); text && text->isString() && !text->asString().empty()) {
        out += " \"" + text->asString() + "\"";
    }
    if (auto ref = node.containsOpt("ref"); ref && ref->isString()) {
        out += "  [" + ref->asString() + "]";
    }
    if (auto visible = node.containsOpt("visible"); visible && visible->isBool() && !visible->asBool()) {
        out += "  (hidden)";
    }
    out += "\n";
    if (auto children = node.containsOpt("children"); children && children->isArray()) {
        for (const auto& child : children->asArray()) {
            renderTree(child, depth + 1, out);
        }
    }
}

}   // namespace

MainWindow::MainWindow() : AWindow("AUI Remote", 900_dp, 600_dp) {
    mAppList = Vertical {} AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp } };
    mAppActions = Horizontal {} AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } };
    mScreenshot = _new<ADrawableView>();
    mSkillCheckBox = _new<ACheckBox>(Label { "Let Agents drive AUI remote" });
    mStatus = "Start an application with AUI_REMOTE=1 and it will show up here.";

    setContents(Vertical {
      Horizontal::Expanding {
        Vertical {
          Label { "Running applications" } AUI_OVERRIDE_STYLE { FontSize { 12_pt } },
          mAppList,
          _new<AButton>("Refresh").connect(&AView::clicked, this, [this] { refreshApps(); }),
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, MinSize { 220_dp, 0 } },
        Vertical::Expanding {
          _new<ALabel>() & mAppTitle AUI_OVERRIDE_STYLE { FontSize { 12_pt } },
          mAppActions,
          Horizontal::Expanding {
            AScrollArea::Builder().withExpanding().withContents(_new<ALabel>() & mTree).build(),
            Centered::Expanding { mScreenshot } AUI_OVERRIDE_STYLE { MinSize { 300_dp, 0 } },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
      Horizontal {
        mSkillCheckBox,
        SpacerExpanding {},
        _new<ALabel>() & mStatus,
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, Padding { 8_dp } });

    connect(mSkillCheckBox->box()->userCheckedChange, this, [this](bool wanted) { skillToggled(wanted); });
    syncSkillCheckBox();

    mTimer = _new<ATimer>(2s);
    connect(mTimer->fired, this, [this] { refreshApps(); });
    mTimer->start();
    refreshApps();
}

MainWindow::~MainWindow() = default;

AOptional<AppInfo> MainWindow::selectedApp() const {
    if (!*mSelectedPid) {
        return std::nullopt;
    }
    for (const auto& app : *mApps) {
        if (app.pid == **mSelectedPid) {
            return app;
        }
    }
    return std::nullopt;
}

bool MainWindow::isPaired(const AppInfo& app) const { return KeyStore().load(app).hasValue(); }

void MainWindow::refreshApps() {
    mAsync << AUI_THREADPOOL {
        auto apps = discovery::list();
        KeyStore().pruneExcept(apps);
        AUI_UI_THREAD {
            mApps = std::move(apps);
            rebuildAppList();
            rebuildActions();
        };
    };
}

void MainWindow::rebuildAppList() {
    mAppList->removeAllViews();
    if (mApps->empty()) {
        mAppList->addView(Label { "No applications found" });
        return;
    }
    for (const auto& app : *mApps) {
        const bool selected = mSelectedPid->hasValue() && **mSelectedPid == app.pid;
        AString title = app.displayName();
        title += isPaired(app) ? "  paired" : "  not paired";
        auto label = _new<ALabel>(title);
        label->setAssName("selected", selected);
        if (selected) {
            label->setCustomStyle({ BackgroundSolid { AColor::BLUE.transparentize(0.6f) }, Padding { 4_dp } });
        } else {
            label->setCustomStyle({ Padding { 4_dp } });
        }
        connect(label->clicked, this, [this, pid = app.pid] { select(pid); });
        mAppList->addView(label);
    }
}

void MainWindow::rebuildActions() {
    mAppActions->removeAllViews();
    auto app = selectedApp();
    if (!app) {
        mAppTitle = "Select an application";
        return;
    }
    mAppTitle = "{}, port {}"_format(app->displayName(), app->port);
    if (isPaired(*app)) {
        mAppActions->addView(
            _new<AButton>("Inspect").connect(&AView::clicked, this, [this] { inspect(); }));
    } else {
        mAppActions->addView(_new<AButton>("Pair...").connect(&AView::clicked, this, [this] { pair(); }));
    }
}

void MainWindow::select(std::uint32_t pid) {
    mSelectedPid = pid;
    mTree = "";
    mScreenshot->setDrawable(nullptr);
    rebuildAppList();
    rebuildActions();
    if (auto app = selectedApp(); app && isPaired(*app)) {
        inspect();
    }
}

void MainWindow::pair() {
    auto app = selectedApp();
    if (!app) {
        return;
    }
    auto dialog = _new<PairingDialog>(*this, *app);
    connect(dialog->paired, this, [this] {
        mStatus = "Paired.";
        refreshApps();
        inspect();
    });
    dialog->show();
}

void MainWindow::inspect() {
    auto app = selectedApp();
    if (!app || mBusy) {
        return;
    }
    auto key = KeyStore().load(*app);
    if (!key) {
        return;
    }
    mBusy = true;
    mStatus = "Looking at {}..."_format(app->displayName());
    mAsync << AUI_THREADPOOL {
        AString tree;
        AString error;
        _<AImage> image;
        try {
            RpcClient client(app->port, *key);
            const auto json = client.call("view.tree", AJson { { "depth", 12 } });
            renderTree(json, 0, tree);
            const auto shot = client.call("screenshot");
            image = AImage::fromFile(APath(shot["path"].asString()));
        } catch (const RpcError& e) {
            error = e.getMessage();
            if (auto hint = e.data().isObject() ? e.data().containsOpt("hint") : std::nullopt; hint && hint->isString()) {
                error += ". " + hint->asString();
            }
        } catch (const AException& e) {
            error = e.getMessage();
        }
        AUI_UI_THREAD {
            mBusy = false;
            if (!error.empty()) {
                mStatus = error;
                return;
            }
            mTree = tree;
            mStatus = "Updated.";
            if (image) {
                mScreenshot->setDrawable(_new<AImageDrawable>(image));
            }
        };
    };
}

void MainWindow::syncSkillCheckBox() { mSkillCheckBox->checked() = skill::isInstalled(); }

void MainWindow::skillToggled(bool wanted) {
    try {
        if (wanted) {
            skill::install(skill::builtinTemplate(), AProcess::self()->getPathToExecutable());
            mStatus = "Skill installed to {}"_format(skill::defaultFile());
        } else {
            skill::uninstall();
            mStatus = "Skill removed.";
        }
    } catch (const AException& e) {
        mStatus = "Can't change the skill: " + e.getMessage();
    }
    syncSkillCheckBox();
}
