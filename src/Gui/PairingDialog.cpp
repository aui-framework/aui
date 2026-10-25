#include "PairingDialog.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ATextField.h>

#include "Remote/KeyStore.h"
#include "Remote/RpcClient.h"

using namespace declarative;
using namespace aui_remote;

namespace {

AString describe(const RpcError& e) {
    AString result = e.getMessage();
    if (e.data().isObject()) {
        if (auto left = e.data().containsOpt("attemptsLeft"); left && left->isInt()) {
            result += left->asInt() > 0 ? " ({} attempts left)"_format(left->asInt())
                                        : " (the code is void, a new one is shown in the application)";
        }
    }
    return result;
}

}   // namespace

PairingDialog::PairingDialog(AWindow& parent, AppInfo app)
  : AWindow(AWindow::Init {
      .title = "Pair with " + app.displayName(),
      .width = 380_dp,
      .height = 170_dp,
      .parent = AWindow::Init::Parent { .window = parent, .modal = true },
      .resizable = false,
    }),
    mApp(std::move(app)) {
    mStatus = "Asking the application to show a code...";
    setContents(Vertical {
      Label { "Enter the 6-digit code shown in the window of the application." },
      _new<ATextField>() && mCode,
      _new<ALabel>() & mStatus,
      Horizontal {
        SpacerExpanding {},
        _new<AButton>("Cancel").connect(&AView::clicked, this, [this] { close(); }),
        _new<AButton>("Pair").connect(&AView::clicked, this, [this] { confirm(); }),
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 8_dp } });
    begin();
}

void PairingDialog::begin() {
    mAsync << AUI_THREADPOOL {
        AString status;
        try {
            RpcClient::forApp(mApp, "").call("pair.begin");
            status = "Waiting for the code.";
        } catch (const RpcError& e) {
            status = "Can't start pairing: " + e.getMessage();
        }
        AUI_UI_THREAD { mStatus = status; };
    };
}

void PairingDialog::confirm() {
    if (mBusy) {
        return;
    }
    mBusy = true;
    mStatus = "Checking the code...";
    mAsync << AUI_THREADPOOL {
        const auto code = (*mCode).trim();
        const auto key = KeyStore::generateKey();
        AString status;
        bool ok = false;
        bool needNewCode = false;
        try {
            RpcClient::forApp(mApp, "").call("pair.confirm", AJson { { "code", code }, { "key", key } });
            KeyStore().store(mApp, key);
            ok = true;
        } catch (const RpcError& e) {
            status = describe(e);
            needNewCode = e.data().isObject() && e.data().containsOpt("attemptsLeft") &&
                          (*e.data().containsOpt("attemptsLeft")).isInt() &&
                          (*e.data().containsOpt("attemptsLeft")).asInt() <= 0;
        } catch (const AException& e) {
            status = e.getMessage();
        }
        AUI_UI_THREAD {
            mBusy = false;
            if (ok) {
                emit paired;
                close();
                return;
            }
            mStatus = status;
            if (needNewCode) {
                begin();
            }
        };
    };
}
