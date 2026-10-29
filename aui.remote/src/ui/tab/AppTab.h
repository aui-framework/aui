#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AViewContainer.h>

#include "network/Discovery.h"
#include "ui/view/RemoteViewport.h"
#include "ui/view/ViewDetails.h"
#include "ui/view/ViewTreeView.h"
#include "network/FrameStream.h"

/**
 * @brief The page of a single connection: pairing, the live view of one application (frames, input and the view tree).
 */
class AppTab : public AViewContainer {
public:
    enum class State {
        CONNECTING,
        NEEDS_PAIRING,
        READY,
        ERROR,
        EXITED,
    };

    explicit AppTab(aui_remote::AppInfo app);
    ~AppTab() override;

    [[nodiscard]] const aui_remote::AppInfo& app() const noexcept { return mApp; }
    [[nodiscard]] const AProperty<State>& state() const noexcept { return mState; }

    /**
     * @brief `Local / myapp (1234)`
     */
    [[nodiscard]] AString title() const;

    /**
     * @brief Tells the tab whether the application is still listed by discovery.
     */
    void setAlive(bool alive);

    /**
     * @brief (Re)connects the frame stream.
     */
    void inspect();

private:
    aui_remote::AppInfo mApp;
    AProperty<State> mState = State::CONNECTING;
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
    _<ViewDetails> mDetails;
    _<const aui_remote::ViewTree> mTree;
    AAsyncHolder mAsync;
    _<aui_remote::FrameStream> mStream;

    void setState(State state);
    void showBanner(const AString& text);
    void startStream();
    void stopStream();
    void pair();
    void setTree(_<const aui_remote::ViewTree> tree);
    void updateHighlight();
    void inspectAt(glm::vec2 windowPos);
    void showDetails(const AString& ref);
    void showRemoteSize(glm::ivec2 size);
    void addScaleOption(float factor);
    void showRemoteScale(float factor);
    void rescaleRemoteWindow(int index);
    void resizeRemoteWindow();
};
