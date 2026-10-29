#pragma once

#include <chrono>

#include <AUI/Common/ATimer.h>
#include <AUI/Image/AImage.h>
#include <AUI/Render/ITexture.h>
#include <AUI/View/AView.h>

#include "ViewportMapping.h"
#include "network/FrameStream.h"

/**
 * @brief Draws the latest frame of the controlled application and forwards the input to it.
 * @details
 * The view takes the logical size of the remote window, so inside ARulerArea it is shown 1:1 and the rulers measure
 * remote window pixels. When it is given a different size, the frame is letterboxed with the aspect ratio preserved.
 *
 * Hover, press, move, release, scroll, keys and characters are sent as `input.pointer`, `input.key` and `input.type`.
 * Press and release are always separate events (never the atomic `click`), so drag works. A finger (T mode) held still
 * for a while also sends `input.gesture` longPress: injected touches don't go through the platform's gesture detector.
 *
 * Like the in-process devtools, the viewport draws the box model (content, padding, margin) of a highlighted view over
 * the frame, and moving the pointer with CTRL held emits inspectRequested (hit testing is up to the receiver).
 */
class RemoteViewport : public AView {
public:
    enum class Pointer {
        MOUSE,
        FINGER,
    };

    RemoteViewport();
    ~RemoteViewport() override;

    /**
     * @brief Where the input goes. Null detaches the viewport (the input is dropped).
     */
    void setStream(_<aui_remote::FrameStream> stream);

    /**
     * @brief Shows the next frame.
     */
    void setFrame(_<AImage> image, glm::ivec2 windowSize);

    /**
     * @brief Drops the picture.
     */
    void clearFrame();

    void setPointer(Pointer pointer);
    [[nodiscard]] Pointer pointer() const noexcept { return mPointer; }

    [[nodiscard]] aui_remote::ViewportMapping mapping() const noexcept;

    /**
     * @brief A view to draw the box model of, in window coordinates.
     */
    struct Highlight {
        aui_remote::ViewTree::Box box;
        AString label;
    };

    /**
     * @brief Draws the box model of the view over the frame; nullopt removes it.
     */
    void setHighlight(AOptional<Highlight> highlight);

    void render(ARenderContext ctx) override;
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void onScroll(const AScrollEvent& event) override;
    void onKeyDown(AInput::Key key) override;
    void onKeyUp(AInput::Key key) override;
    void onCharEntered(AChar c) override;
    bool handlesNonMouseNavigation() override { return true; }

signals:
    emits<Pointer> pointerChanged;

    /**
     * @brief The pointer is moved over the frame with CTRL held. The argument is the window position.
     */
    emits<glm::vec2> inspectRequested;

protected:
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override;

private:
    _<aui_remote::FrameStream> mStream;
    _<AImage> mImage;
    _<ITexture> mTexture;
    bool mTextureDirty = false;
    glm::ivec2 mWindowSize { 0, 0 };
    Pointer mPointer = Pointer::MOUSE;
    bool mPressed = false;
    _<ATimer> mMoveTimer;
    _<ATimer> mLongPressTimer;
    AOptional<glm::vec2> mLongPressAt;   // window position of the finger that may become a long press
    AOptional<glm::vec2> mPendingMove;
    std::chrono::steady_clock::time_point mLastMove {};
    AOptional<Highlight> mHighlight;

    void sendPointer(const char* action, glm::vec2 windowPos, AOptional<AInput::Key> button = std::nullopt);
    void sendMove(glm::vec2 windowPos);
    void flushMove();
    void cancelLongPress();
    void fireLongPress();
    void renderHighlight(ARenderContext& ctx, const Highlight& highlight);
};
