#include "AppTab.h"

#include <AUI/Platform/AWindow.h>
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

#include "PairingDialog.h"
#include "RemoteViewport.h"
#include "remote/KeyStore.h"
#include "remote/RpcClient.h"

using namespace declarative;
using namespace aui_remote;

AppTab::AppTab(AppInfo app) : mApp(std::move(app)) {
    mPointer = mApp.defaultPointer == "finger" ? RemoteViewport::Pointer::FINGER : RemoteViewport::Pointer::MOUSE;
    mViewport = _new<RemoteViewport>();
    connect(mPointer, AUI_SLOT(mViewport)::setPointer);

    auto pointerButton = [this](AString text, RemoteViewport::Pointer pointer) {
        auto button = Button {
            .content = Label { std::move(text) },
            .onClick = [this, pointer] { mPointer = pointer; },
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
        } AUI_OVERRIDE_STYLE { BoxShadow { 0, 4_dp, 16_dp, AColor::GRAY.transparentize(0.5f) } }),
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
    mDetails = _new<ViewDetails>();
    connect(mTreeView->hovered.changed, me::updateHighlight);
    connect(mTreeView->selected.changed, [this](const AString& ref) {
        updateHighlight();
        showDetails(ref);
    });
    connect(mViewport->inspectRequested, me::inspectAt);

    mTreeView->setExpanding();
    mDetails->setExpanding();
    auto inspector = ASplitter::Vertical()
        .withExpanding()
        .withItems({ mTreeView, mDetails })
        .build();
    inspector AUI_OVERRIDE_STYLE { MinSize { 200_dp, 0 } };

    // the splitter takes the expanding of its items as is, so it's set before the construction
    viewportArea->setExpanding();
    auto main = ASplitter::Horizontal().withExpanding().withItems({
        viewportArea,
        inspector,
    }).build();

    setContents(Vertical {
      Horizontal {
        Label { "{}, port {}"_format(mApp.displayName(), mApp.port) } AUI_OVERRIDE_STYLE { FontSize { 12_pt } },
        _new<ASpinnerV2>() AUI_LET {
            connect(AUI_REACT(*mBusy ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
        },
        SpacerExpanding {},
        Button { .content = Label { "Reconnect" }, .onClick = [this] { inspect(); } },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
      Horizontal {
        Label { AUI_REACT(*mBanner) } AUI_OVERRIDE_STYLE { TextColor { AColor::RED } },
        SpacerExpanding {},
        Button { .content = Label { "Retry" }, .onClick = [this] { inspect(); } },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, Padding { 6_dp } } AUI_LET {
          connect(
              AUI_REACT(*mState == State::ERROR || *mState == State::EXITED ? Visibility::VISIBLE : Visibility::GONE),
              it->visibility());
      },
      Centered {
        Vertical {
          Label { "This application needs to confirm the access" } AUI_OVERRIDE_STYLE { FontSize { 12_pt } },
          Label { "It will show a 6-digit code. The connection is encrypted and the certificate is pinned." },
          Button { .content = Label { "Pair..." }, .onClick = [this] { pair(); } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
      } AUI_LET {
          it->setExpanding();
          connect(AUI_REACT(*mState == State::NEEDS_PAIRING ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
      },
      main AUI_LET {
          connect(
              AUI_REACT(
                  *mState == State::NEEDS_PAIRING ? Visibility::GONE : Visibility::VISIBLE),
              it->visibility());
      },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 8_dp } });

    if (KeyStore().load(mApp)) {
        inspect();
    } else {
        setState(State::NEEDS_PAIRING);
    }
}

AppTab::~AppTab() { stopStream(); }

AString AppTab::title() const { return "Local / " + mApp.displayName(); }

void AppTab::setAlive(bool alive) {
    if (!alive && *mState != State::EXITED) {
        stopStream();
        showBanner("The application has exited.");
        setState(State::EXITED);
    }
}

void AppTab::setState(State state) {
    if (*mState != state) {
        mState = state;
    }
}

void AppTab::showBanner(const AString& text) { mBanner = text; }

void AppTab::pair() {
    auto window = dynamic_cast<AWindow*>(getWindow());
    if (!window) {
        return;
    }
    auto dialog = _new<PairingDialog>(*window, mApp);
    connect(dialog->paired, this, [this] { inspect(); });
    dialog->show();
}

void AppTab::stopStream() {
    mViewport->setStream(nullptr);
    if (mStream) {
        mStream->stop();
        // may be called from a slot of the stream itself, so it outlives the emission
        AThread::current()->enqueue([old = std::move(mStream)] {});
        mStream = nullptr;
    }
    mBusy = false;
}

void AppTab::inspect() {
    if (*mState == State::EXITED) {
        return;
    }
    startStream();
}

void AppTab::startStream() {
    auto key = KeyStore().load(mApp);
    if (!key) {
        stopStream();
        setState(State::NEEDS_PAIRING);
        return;
    }
    stopStream();
    mBusy = true;
    setState(State::CONNECTING);

    mStream = _new<FrameStream>(mApp, *key);
    mViewport->setStream(mStream);
    const auto* self = mStream.get();   // the signals of a replaced stream are ignored
    connect(mStream->frameReceived, this, [this, self](const FrameUpdate& frame) {
        if (mStream.get() != self || *mState == State::EXITED) {
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
        if (*mState == State::READY) {
            return;
        }
        setState(State::READY);
    });
    connect(mStream->failed, this, [this, self](const AString& message) {
        if (mStream.get() != self) {
            return;
        }
        const bool untrusted = mStream && mStream->lastErrorCode() == rpc_code::UNAUTHORIZED;
        stopStream();
        if (*mState == State::EXITED) {
            return;
        }
        if (untrusted) {
            // the application has forgotten this client (it asks for the code after a restart)
            KeyStore().remove(mApp);
            setState(State::NEEDS_PAIRING);
            return;
        }
        showBanner(message);
        setState(State::ERROR);
    });
    mStream->start();
}

void AppTab::setTree(_<const ViewTree> tree) {
    mTree = std::move(tree);
    mTreeView->setTree(mTree);
    updateHighlight();
}

void AppTab::updateHighlight() {
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

void AppTab::inspectAt(glm::vec2 windowPos) {
    if (!mTree) {
        return;
    }
    if (auto path = mTree->hitTest(windowPos); !path.empty()) {
        mTreeView->select(path.back());
    }
}

void AppTab::showDetails(const AString& ref) {
    const auto* fields = mTree ? mTree->fields(ref) : nullptr;
    if (!fields) {
        mDetails->clear();
        return;
    }
    mDetails->show(ref, *fields);
    if (!mStream) {
        return;
    }
    // the response comes on a worker thread
    auto self = _weak<AppTab>(aui::ptr::shared_from_this(this));
    mAsync << mStream->request("view.properties", AJson { { "ref", ref } })
                  .onSuccess([self, ref](const AJson& properties) {
                      AThread::main()->enqueue([self, ref, properties] {
                          if (auto tab = self.lock()) {
                              tab->mDetails->setProperties(ref, properties);
                          }
                      });
                  })
                  .onError([self, ref](const AException& e) {
                      AThread::main()->enqueue([self, ref, message = e.getMessage()] {
                          if (auto tab = self.lock()) {
                              tab->mDetails->setPropertiesError(ref, message);
                          }
                      });
                  });
}

void AppTab::showRemoteSize(glm::ivec2 size) {
    mRemoteSize = size;
    // a value being typed is not overwritten
    if (!mWidthField->hasFocus()) {
        mWidthField->setText(AString::number(size.x));
    }
    if (!mHeightField->hasFocus()) {
        mHeightField->setText(AString::number(size.y));
    }
}

void AppTab::resizeRemoteWindow() {
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
                  .onError([](const AException& e) { ALogger::warn("AppTab") << "window.resize: " << e; });
}

void AppTab::addScaleOption(float factor) {
    mScaleFactors << factor;
    mScaleModel->push_back("{}%"_format(int(std::round(factor * 100.f))));
}

void AppTab::showRemoteScale(float factor) {
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

void AppTab::rescaleRemoteWindow(int index) {
    if (!mStream || index < 0 || index >= int(mScaleFactors.size())) {
        return;
    }
    const auto factor = mScaleFactors[index];
    if (std::abs(factor - mRemoteScale) < 0.001f) {
        return;
    }
    mAsync << mStream->request("window.setScale", AJson { { "factor", double(factor) } })
                  .onError([](const AException& e) { ALogger::warn("AppTab") << "window.setScale: " << e; });
}
