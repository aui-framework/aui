#include "AppTab.h"

#include <AUI/Logging/ALogger.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/ARulerArea.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ASpinnerV2.h>
#include <AUI/View/ASpacerFixed.h>
#include <AUI/View/ASplitter.h>
#include <AUI/View/ATextField.h>

#include "ui/view/RemoteViewport.h"
#include "ui/view/ViewDetails.h"
#include "ui/view/ViewTreeView.h"
#include "network/FrameStream.h"
#include "network/KeyStore.h"
#include "network/RpcClient.h"

using namespace declarative;
using namespace aui_remote;

namespace {
_<AView> heading(AString text) { return Label { std::move(text) } AUI_OVERRIDE_STYLE { FontSize { 12_pt } }; }
using Tab = AppTabModel::State;

class AppTab : public AObject {
public:
    AppTab(_<AppTabModel> model, PairRequest pair) : mModel(std::move(model)), mApp(mModel->app), mState(mModel->state), mPair(std::move(pair)) {}

    ~AppTab() override { stopStream(); }

    _<AView> build() {
        mPointer = mApp.defaultPointer == "finger" ? RemoteViewport::Pointer::FINGER : RemoteViewport::Pointer::MOUSE;
        mViewport = _new<RemoteViewport>();
        connect(mPointer, AUI_SLOT(mViewport)::setPointer);

        auto pointerButton = [this](AString text, RemoteViewport::Pointer pointer) {
            auto button = Button {
                Label { std::move(text) },
                [this, pointer] { mPointer = pointer; },
            }();
            connect(AUI_REACT(*mPointer == pointer), AUI_SLOT(button)::setDefault);
            return button;
        };

        auto sizeField = [this] {
            auto field = _new<ATextField>();
            connect(field->actionButtonPressed, me::resizeRemoteWindow);
            connect(field->textChanged, me::resizeRemoteWindow);   // editing finished
            return field AUI_OVERRIDE_STYLE { FixedSize { 56_dp, {} } };
        };
        mWidthField = sizeField();
        mHeightField = sizeField();

        mScaleModel = _new<AListModel<AString>>();
        for (auto factor : { 0.5f, 0.75f, 1.f, 1.25f, 1.5f, 2.f, 3.f }) {
            addScaleOption(factor);
        }
        mScaleList = _new<ADropdownList>(mScaleModel);
        mScaleList->setSelectionId(2);
        connect(mScaleList->selectionChanged, me::rescaleRemoteWindow);

        // the viewport inside the ruler area, with the remote window size and the mode switch [ M | T ] in the corner
        _<AView> viewportArea = Stacked::Expanding {
            _new<ARulerArea>(Stacked {
                mViewport
            } AUI_OVERRIDE_STYLE { BoxShadow { 0, 4_dp, 16_dp, AColor::GRAY.transparentize(0.5f) } }) AUI_LET {
                it->setFitContents(true);
            },
            Vertical::Expanding {
              Horizontal {
                SpacerExpanding {},
                mWidthField,
                Label { "×" },
                mHeightField,
                SpacerFixed { 8_dp },
                mScaleList,
                SpacerFixed { 8_dp },
                pointerButton("M", RemoteViewport::Pointer::MOUSE),
                pointerButton("T", RemoteViewport::Pointer::FINGER),
              } AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Margin { 24_dp, 8_dp, 0, 0 } },
            },
        };

        mTreeView = _new<ViewTreeView>();
        mDetails = _new<AProperty<ViewDetailsModel>>();
        auto details = viewDetails(mDetails);
        connect(mTreeView->hovered.changed, me::updateHighlight);
        connect(mTreeView->selected.changed, [this](const AString& ref) {
            updateHighlight();
            showDetails(ref);
        });
        connect(mViewport->inspectRequested, me::inspectAt);

        mTreeView->setExpanding();
        details->setExpanding();
        auto inspector = ASplitter::Vertical()
            .withExpanding()
            .withItems({ mTreeView, details })
            .build();
        inspector AUI_OVERRIDE_STYLE { MinSize { 200_dp, 0 } };

        // the splitter takes the expanding of its items as is, so it's set before the construction
        viewportArea->setExpanding();
        auto main = ASplitter::Horizontal().withExpanding().withItems({
            viewportArea,
            inspector,
        }).build();

        return Vertical {
          Horizontal {
            heading("{}, port {}"_format(mApp.displayName(), mApp.port)),
            _new<ASpinnerV2>() AUI_LET {
                connect(AUI_REACT(*mBusy ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
            },
            SpacerExpanding {},
            Button { Label { "Reconnect" }, [this] { inspect(); } },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
          Horizontal {
            Label { AUI_REACT(*mBanner) } AUI_OVERRIDE_STYLE { TextColor { AColor::RED } },
            SpacerExpanding {},
            Button { Label { "Retry" }, [this] { inspect(); } },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, Padding { 6_dp } } AUI_LET {
              connect(
                  AUI_REACT(*mState == Tab::FAILED || *mState == Tab::EXITED ? Visibility::VISIBLE : Visibility::GONE),
                  it->visibility());
          },
          Centered {
            Vertical {
              heading("This application needs to confirm the access"),
              Label { "It will show a 6-digit code. The connection is encrypted and the certificate is pinned." },
              Button { Label { "Pair..." }, [this] { pair(); } },
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
          } AUI_LET {
              it->setExpanding();
              connect(AUI_REACT(*mState == Tab::NEEDS_PAIRING ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
          },
          main AUI_LET {
              connect(
                  AUI_REACT(
                      *mState == Tab::NEEDS_PAIRING ? Visibility::GONE : Visibility::VISIBLE),
                  it->visibility());
          },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 8_dp } };
    }

    void start() {
        if (KeyStore().load(mApp)) {
            inspect();
        } else {
            setState(Tab::NEEDS_PAIRING);
        }
    }

    void setAlive(bool alive) {
    if (!alive && *mState != Tab::EXITED) {
        stopStream();
        showBanner("The application has exited.");
        setState(Tab::EXITED);
    }
}

    void setState(Tab state) {
    if (*mState != state) {
        mState = state;
    }
}

    void setDetailsProperties(const AString& ref, const AJson& properties) {
        if ((*mDetails)->ref == ref) {
            mDetails->writeScope()->properties = properties;
        }
    }

    void setDetailsError(const AString& ref, const AString& message) {
        if ((*mDetails)->ref == ref) {
            mDetails->writeScope()->propertiesError = message;
        }
    }

    void showBanner(const AString& text) { mBanner = text; }

    void pair() {
    mPair(mApp, [self = _weak<AppTab>(aui::ptr::shared_from_this(this))] {
        if (auto tab = self.lock()) {
            tab->inspect();
        }
    });
}

    void stopStream() {
    mViewport->setStream(nullptr);
    if (mStream) {
        mStream->stop();
        // may be called from a slot of the stream itself, so it outlives the emission
        AThread::current()->enqueue([old = std::move(mStream)] {});
        mStream = nullptr;
    }
    mBusy = false;
}

    void inspect() {
    if (*mState == Tab::EXITED) {
        return;
    }
    startStream();
}

    void startStream() {
    auto key = KeyStore().load(mApp);
    if (!key) {
        stopStream();
        setState(Tab::NEEDS_PAIRING);
        return;
    }
    stopStream();
    mBusy = true;
    setState(Tab::CONNECTING);

    mStream = _new<FrameStream>(mApp, *key);
    mViewport->setStream(mStream);
    const auto* self = mStream.get();   // the signals of a replaced stream are ignored
    connect(mStream->frameReceived, this, [this, self](const FrameUpdate& frame) {
        if (mStream.get() != self || *mState == Tab::EXITED) {
            return;
        }
        if (frame.image) {
            mViewport->setFrame(frame.image, frame.windowSize);
            showRemoteSize(frame.windowSize);
            showRemoteScale(frame.scalingFactor);
        }
        if (frame.tree && !frame.tree->empty()) {
            setTree(frame.tree);
        }
        mBusy = false;
        if (*mState == Tab::READY) {
            return;
        }
        setState(Tab::READY);
    });
    connect(mStream->failed, this, [this, self](const AString& message) {
        if (mStream.get() != self) {
            return;
        }
        const bool untrusted = mStream && mStream->lastErrorCode() == rpc_code::UNAUTHORIZED;
        stopStream();
        if (*mState == Tab::EXITED) {
            return;
        }
        if (untrusted) {
            // the application has forgotten this client (it asks for the code after a restart)
            KeyStore().remove(mApp);
            setState(Tab::NEEDS_PAIRING);
            return;
        }
        showBanner(message);
        setState(Tab::FAILED);
    });
    mStream->start();
}

    void setTree(_<const ViewTree> tree) {
    mTree = std::move(tree);
    mTreeView->setTree(mTree);
    updateHighlight();
}

    void updateHighlight() {
    const auto& ref = mTreeView->hovered->empty() ? *mTreeView->selected : *mTreeView->hovered;
    const auto* fields = mTree ? mTree->fields(ref) : nullptr;
    if (!fields) {
        mViewport->setHighlight(std::nullopt);
        return;
    }
    mViewport->setHighlight(RemoteViewport::Highlight {
      .box = ViewTree::boxOf(*fields),
      .label = ViewTree::prettyName(*fields),
    });
}

    void inspectAt(glm::vec2 windowPos) {
    if (!mTree) {
        return;
    }
    if (auto path = mTree->hitTest(windowPos); !path.empty()) {
        mTreeView->select(path.back());
    }
}

    void showDetails(const AString& ref) {
    const auto* fields = mTree ? mTree->fields(ref) : nullptr;
    if (!fields) {
        *mDetails = ViewDetailsModel {};
        return;
    }
    *mDetails = ViewDetailsModel { .ref = ref, .fields = *fields };
    if (!mStream) {
        return;
    }
    // the response comes on a worker thread
    auto self = _weak<AppTab>(aui::ptr::shared_from_this(this));
    mAsync << mStream->request("view.properties", AJson { { "ref", ref } })
                  .onValue([self, ref](const AJson& properties) {
                      AThread::main()->enqueue([self, ref, properties] {
                          if (auto tab = self.lock()) {
                              tab->setDetailsProperties(ref, properties);
                          }
                      });
                  })
                  .onException([self, ref](const AException& e) {
                      AThread::main()->enqueue([self, ref, message = e.getMessage()] {
                          if (auto tab = self.lock()) {
                              tab->setDetailsError(ref, message);
                          }
                      });
                  });
}

    void showRemoteSize(glm::ivec2 size) {
    // a value being typed is not overwritten; a focused field that shows the old size is not being typed into, and
    // would send that size again when it loses focus
    const auto update = [](ATextField& field, int oldValue, int value) {
        if (!field.hasFocus() || field.text() == AString::number(oldValue)) {
            field.setText(AString::number(value));
        }
    };
    const auto oldSize = std::exchange(mRemoteSize, size);
    update(*mWidthField, oldSize.x, size.x);
    update(*mHeightField, oldSize.y, size.y);
}

    void resizeRemoteWindow() {
    const auto width = mWidthField->text()->trim().toInt();
    const auto height = mHeightField->text()->trim().toInt();
    if (!mStream || !width || !height || *width <= 0 || *height <= 0) {
        showRemoteSize(mRemoteSize);
        return;
    }
    const glm::ivec2 size { *width, *height };
    if (size == mRemoteSize) {
        return;
    }
    // the next frame shows the size the window got, which may differ (its minimum size, the window manager)
    mAsync << mStream->request("window.resize", AJson { { "width", size.x }, { "height", size.y } })
                  .onException([](const AException& e) { ALogger::warn("AppTab") << "window.resize: " << e; });
}

    void addScaleOption(float factor) {
    mScaleFactors << factor;
    mScaleModel->push_back("{}%"_format(int(std::round(factor * 100.f))));
}

    void showRemoteScale(float factor) {
    mRemoteScale = factor;
    auto it = std::find_if(mScaleFactors.begin(), mScaleFactors.end(),
                           [&](float f) { return std::abs(f - factor) < 0.001f; });
    if (it == mScaleFactors.end()) {
        // set by someone else (i.e. the CLI) to a value that is not in the list
        addScaleOption(factor);
        it = mScaleFactors.end() - 1;
    }
    // equal to mRemoteScale, so rescaleRemoteWindow does not send it back
    mScaleList->setSelectionId(int(it - mScaleFactors.begin()));
}

    void rescaleRemoteWindow(int index) {
    if (!mStream || index < 0 || index >= int(mScaleFactors.size())) {
        return;
    }
    const auto factor = mScaleFactors[index];
    if (std::abs(factor - mRemoteScale) < 0.001f) {
        return;
    }
    mAsync << mStream->request("window.setScale", AJson { { "factor", double(factor) } })
                  .onException([](const AException& e) { ALogger::warn("AppTab") << "window.setScale: " << e; });
}

private:
    _<AppTabModel> mModel;
    const AppInfo& mApp;
    AProperty<Tab>& mState;
    PairRequest mPair;
    AProperty<AString> mBanner;
    AProperty<bool> mBusy = false;   // connecting
    AProperty<RemoteViewport::Pointer> mPointer = RemoteViewport::Pointer::MOUSE;
    _<RemoteViewport> mViewport;
    _<ATextField> mWidthField;
    _<ATextField> mHeightField;
    glm::ivec2 mRemoteSize { 0, 0 };
    _<ADropdownList> mScaleList;
    _<AListModel<AString>> mScaleModel;
    AVector<float> mScaleFactors;
    float mRemoteScale = 1.f;
    _<ViewTreeView> mTreeView;
    _<AProperty<ViewDetailsModel>> mDetails;
    _<const ViewTree> mTree;
    AAsyncHolder mAsync;
    _<FrameStream> mStream;
};

}   // namespace

_<AView> appTab(_<AppTabModel> model, PairRequest pair) {
    auto tab = _new<AppTab>(model, std::move(pair));
    auto root = tab->build();
    // the root owns the tab: views refer to it by raw pointers, a strong reference from them would never be released
    AObject::connect(model->alive.changed, root, [tab](bool alive) { tab->setAlive(alive); });
    tab->start();
    return root;
}
