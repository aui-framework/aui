#include "PairingDialog.h"

#include <AUI/Common/AProperty.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ATextField.h>

#include "network/KeyStore.h"
#include "network/Pairing.h"
#include "network/RpcClient.h"

using namespace declarative;
using namespace aui_remote;

namespace {

AString describe(const RpcError& e) {
    AString result = e.getMessage();
    if (e.data().isObject()) {
        if (auto left = e.data().containsOpt("attemptsLeft"); left && left->isInt()) {
            result += left->asInt() > 0 ? " (attempts left: {})"_format(left->asInt())
                                        : " (the code is void, a new one is shown in the application)";
        }
    }
    return result;
}

struct State {
    AppInfo app;
    std::function<void()> onPaired;
    AProperty<AString> code;
    AProperty<AString> status = "Asking the application to show a code...";
    AProperty<bool> busy = false;
    AView* field = nullptr;   // a child of the window; the container returned to the caller doesn't stay in it
    AAsyncHolder async;

    void close() {
        if (auto window = dynamic_cast<AWindow*>(field->getWindow())) {
            window->close();
        }
    }

    void begin(AString waitingStatus) {
        async << AUI_THREADPOOL_X[this, waitingStatus = std::move(waitingStatus)] {
            AString result;
            try {
                RpcClient::forApp(app, "").call("pair.begin");
                result = waitingStatus;
            } catch (const RpcError& e) {
                result = "Can't start pairing: " + e.getMessage();
            }
            AUI_UI_THREAD { status = result; };
        };
    }

    void confirm() {
        if (busy) {
            return;
        }
        busy = true;
        status = "Checking the code...";
        async << AUI_THREADPOOL_X[this, code = (*code).trim()] {
            AString result;
            bool ok = false;
            bool needNewCode = false;
            try {
                pairing::confirm(app, code, KeyStore());
                ok = true;
            } catch (const RpcError& e) {
                result = describe(e);
                needNewCode = e.data().isObject() && e.data().containsOpt("attemptsLeft") &&
                              (*e.data().containsOpt("attemptsLeft")).isInt() &&
                              (*e.data().containsOpt("attemptsLeft")).asInt() <= 0;
            } catch (const AException& e) {
                result = e.getMessage();
            }
            AUI_UI_THREAD {
                busy = false;
                if (ok) {
                    onPaired();
                    close();
                    return;
                }
                status = result;
                if (needNewCode) {
                    begin(result);
                }
            };
        };
    }
};

}   // namespace

_<AViewContainer> pairingDialog(AppInfo app, std::function<void()> onPaired) {
    auto state = _new<State>();
    state->app = std::move(app);
    state->onPaired = std::move(onPaired);

    _<AViewContainer> result = Vertical {
      Label { "Enter the 6-digit code shown in the window of the application." },
      (_new<ATextField>() && state->code) AUI_LET {
          AObject::connect(it->actionButtonPressed, it, [state = state.get()] { state->confirm(); });
          it->focus();
          state->field = it.get();
      },
      Label { AUI_REACT(*state->status) },   // owns the state
      Horizontal {
        SpacerExpanding {},
        Button { Label { "Cancel" }, [state = state.get()] { state->close(); } },
        Button { Label { "Pair" }, [state = state.get()] { state->confirm(); } },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 8_dp } };
    state->begin("Waiting for the code.");
    return result;
}
