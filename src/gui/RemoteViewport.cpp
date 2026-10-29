#include "RemoteViewport.h"

#include <AUI/Platform/ACursor.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Render/IRenderer.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Image/IDrawable.h>
#include <AUI/Url/AUrl.h>

using namespace aui_remote;
using namespace std::chrono_literals;

namespace {

constexpr auto MOVE_PERIOD = 16ms;   // ~60 hover events per second at most

// a finger held still this long is a long press; like the default of Android, which reports long presses to AUI
// there. Injected touches don't go through the platform, so the viewport tells the application itself.
constexpr auto LONG_PRESS_TIMEOUT = 500ms;
constexpr float LONG_PRESS_SLOP = 8.f;   // window px the finger may wander

AString keyName(AInput::Key key) {
    auto name = AEnumerate<AInput::Key>::toName(key);
    if (name.startsWith("AInput::")) {
        return name.substr(8);
    }
    return name;
}

AJson modifiers() {
    AJson::Array result;
    for (const auto& [key, name] : { std::pair { AInput::LCONTROL, "ctrl" }, { AInput::LSHIFT, "shift" },
                                     { AInput::LALT, "alt" }, { AInput::LSYSTEM, "cmd" } }) {
        if (AInput::isKeyDown(key)) {
            result << AJson(name);
        }
    }
    return AJson(std::move(result));
}

// the colors of AViewProfiler::displayBoundsOn
const AColor CONTENT_COLOR = 0x7cb6c180u;
const AColor PADDING_COLOR = 0xbccf9180u;
const AColor MARGIN_COLOR = 0xffcca4a0u;

struct Rect {
    glm::vec2 position;
    glm::vec2 size;
};

/// The part of `outer` that is not `inner`.
void frame(IRenderer& render, const ABrush& brush, Rect outer, Rect inner) {
    const auto outerEnd = outer.position + outer.size;
    const auto innerEnd = inner.position + inner.size;
    auto rect = [&](glm::vec2 position, glm::vec2 size) {
        if (size.x > 0 && size.y > 0) {
            render.rectangle(brush, position, size);
        }
    };
    rect(outer.position, { outer.size.x, inner.position.y - outer.position.y });
    rect({ outer.position.x, innerEnd.y }, { outer.size.x, outerEnd.y - innerEnd.y });
    rect({ outer.position.x, inner.position.y }, { inner.position.x - outer.position.x, inner.size.y });
    rect({ innerEnd.x, inner.position.y }, { outerEnd.x - innerEnd.x, inner.size.y });
}

}   // namespace

RemoteViewport::RemoteViewport() {
    mMoveTimer = _new<ATimer>(MOVE_PERIOD);
    connect(mMoveTimer->fired, me::flushMove);
    mLongPressTimer = _new<ATimer>(LONG_PRESS_TIMEOUT);
    connect(mLongPressTimer->fired, me::fireLongPress);
}

void RemoteViewport::cancelLongPress() {
    mLongPressTimer->stop();
    mLongPressAt.reset();
}

void RemoteViewport::fireLongPress() {
    const auto at = mLongPressAt;
    cancelLongPress();
    if (!at || !mStream) {
        return;
    }
    mStream->call("input.gesture", AJson { { "gesture", "longPress" }, { "x", at->x }, { "y", at->y } });
}

RemoteViewport::~RemoteViewport() = default;

void RemoteViewport::setStream(_<FrameStream> stream) {
    mStream = std::move(stream);
    mPressed = false;
    cancelLongPress();
    mPendingMove.reset();
}

void RemoteViewport::setFrame(_<AImage> image, glm::ivec2 windowSize) {
    const bool resized = windowSize != mWindowSize;
    mImage = std::move(image);
    mWindowSize = windowSize;
    mTextureDirty = true;
    if (resized) {
        requestLayout();
    }
    redraw();
}

void RemoteViewport::clearFrame() {
    mImage = nullptr;
    mTexture = nullptr;
    mTextureDirty = false;
    mWindowSize = { 0, 0 };
    requestLayout();
    redraw();
}

void RemoteViewport::setHighlight(AOptional<Highlight> highlight) {
    mHighlight = std::move(highlight);
    redraw();
}

void RemoteViewport::setPointer(Pointer pointer) {
    if (mPointer == pointer) {
        return;
    }
    mPointer = pointer;
    if (pointer == Pointer::FINGER) {
        static const auto cursor = IDrawable::fromUrl(":img/finger.svg");
        setCursor(ACursor(cursor, { .size = 32, .hotspot = ACursor::Hotspot::CENTER }));
    } else {
        setCursor(ACursor(ACursor::DEFAULT));
    }
    emit pointerChanged(pointer);
}

ViewportMapping RemoteViewport::mapping() const noexcept {
    return { glm::vec2(getSize()), glm::vec2(mWindowSize) };
}

glm::ivec2 RemoteViewport::onIntrinsicMeasure(AConstraints) {
    if (mWindowSize.x <= 0 || mWindowSize.y <= 0) {
        return { 320, 240 };
    }
    return mWindowSize;
}

void RemoteViewport::render(ARenderContext ctx) {
    AView::render(ctx);
    if (!mImage) {
        return;
    }
    if (!mTexture) {
        mTexture = ctx.render.getNewTexture();
        mTextureDirty = true;
    }
    if (mTextureDirty) {
        mTexture->setImage(*mImage);
        mTextureDirty = false;
    }
    const auto rect = mapping().content();
    ctx.render.rectangle(ATexturedBrush { .texture = mTexture }, rect.offset, rect.size);
    if (mHighlight) {
        renderHighlight(ctx, *mHighlight);
    }
}

void RemoteViewport::renderHighlight(ARenderContext& ctx, const Highlight& highlight) {
    const auto m = mapping();
    const auto& box = highlight.box;
    // padding and margin are {left, top, right, bottom}
    auto toViewport = [&](glm::vec2 position, glm::vec2 size) {
        return Rect { m.toViewport(position), size * m.scale() };
    };
    const glm::vec2 position = box.position;
    const glm::vec2 size = box.size;
    const glm::vec2 paddingLeftTop { box.padding[0], box.padding[1] };
    const glm::vec2 paddingRightBottom { box.padding[2], box.padding[3] };
    const glm::vec2 marginLeftTop { box.margin[0], box.margin[1] };
    const glm::vec2 marginRightBottom { box.margin[2], box.margin[3] };
    const auto border = toViewport(position, size);
    const auto content = toViewport(
        position + paddingLeftTop, glm::max(size - paddingLeftTop - paddingRightBottom, glm::vec2(0)));
    const auto margin = toViewport(position - marginLeftTop, size + marginLeftTop + marginRightBottom);

    ctx.render.rectangle(ASolidBrush { CONTENT_COLOR }, content.position, content.size);
    frame(ctx.render, ASolidBrush { PADDING_COLOR }, border, content);
    frame(ctx.render, ASolidBrush { MARGIN_COLOR }, margin, border);

    // the label under the margin box, as in-process
    const AFontStyle fs { .size = static_cast<unsigned>(9_pt), .fontRendering = FontRendering::ANTIALIASING };
    const auto lineHeight = float(fs.getLineHeight());
    const AString lines[] = { highlight.label, "{}x{}"_format(box.size.x, box.size.y) };
    const glm::vec2 origin { margin.position.x, margin.position.y + margin.size.y + 2_dp };
    float width = 0;
    for (const auto& line : lines) {
        width = std::max(width, float(fs.getWidth(line)));
    }
    ctx.render.rectangle(
        ASolidBrush { AColor(0x00000070u) }, origin, { width + 4_dp, lineHeight * std::size(lines) + 2_dp });
    RenderHints::PushColor color(ctx.render);
    ctx.render.setColor(AColor::WHITE);
    for (std::size_t i = 0; i < std::size(lines); ++i) {
        ctx.render.string(
            { origin.x + 2_dp, origin.y + 1_dp + float(fs.getAscenderHeight()) + lineHeight * i }, lines[i], fs);
    }
}

void RemoteViewport::sendPointer(const char* action, glm::vec2 windowPos, AOptional<AInput::Key> button) {
    if (!mStream) {
        return;
    }
    AJson params {
        { "action", action },
        { "pointer", mPointer == Pointer::FINGER ? "finger" : "mouse" },
        { "x", windowPos.x },
        { "y", windowPos.y },
    };
    if (mPointer == Pointer::MOUSE && button) {
        params["button"] = *button == AInput::RBUTTON ? "right" : *button == AInput::CBUTTON ? "middle" : "left";
    }
    if (auto m = modifiers(); !m.asArray().empty()) {
        params["modifiers"] = std::move(m);
    }
    mStream->call("input.pointer", std::move(params));
}

void RemoteViewport::sendMove(glm::vec2 windowPos) {
    mPendingMove.reset();
    mMoveTimer->stop();
    mLastMove = std::chrono::steady_clock::now();
    sendPointer("move", windowPos);
}

void RemoteViewport::flushMove() {
    if (mPendingMove) {
        sendMove(*mPendingMove);
    } else {
        mMoveTimer->stop();
    }
}

void RemoteViewport::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AView::onPointerMove(pos, event);
    const auto m = mapping();
    if (AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) {
        if (auto windowPos = m.toWindow(pos)) {
            emit inspectRequested(*windowPos);
        }
    }
    AOptional<glm::vec2> target;
    if (mPressed) {
        target = m.toWindowClamped(pos);   // dragging, keep following the pointer outside the frame
    } else {
        target = m.toWindow(pos);
    }
    if (!target) {
        return;
    }
    if (mLongPressAt && glm::distance(*mLongPressAt, *target) > LONG_PRESS_SLOP) {
        cancelLongPress();
    }
    if (std::chrono::steady_clock::now() - mLastMove >= MOVE_PERIOD) {
        sendMove(*target);
        return;
    }
    mPendingMove = target;   // trailing event so the final position isn't lost
    if (!mMoveTimer->isStarted()) {
        mMoveTimer->start();
    }
}

void RemoteViewport::onPointerPressed(const APointerPressedEvent& event) {
    AView::onPointerPressed(event);
    focus();
    const auto target = mapping().toWindow(event.position);
    if (!target) {
        return;
    }
    mPressed = true;
    sendMove(*target);
    sendPointer("press", *target, event.asButton);
    if (mPointer == Pointer::FINGER) {
        mLongPressAt = *target;
        mLongPressTimer->start();
    }
}

void RemoteViewport::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);
    if (!mPressed) {
        return;
    }
    mPressed = false;
    cancelLongPress();
    const auto target = mapping().toWindowClamped(event.position);
    sendMove(target);   // also flushes the pending move
    sendPointer("release", target, event.asButton);
}

void RemoteViewport::onScroll(const AScrollEvent& event) {
    AView::onScroll(event);
    if (!mStream) {
        return;
    }
    const auto target = mapping().toWindow(event.origin);
    if (!target) {
        return;
    }
    mStream->call("input.pointer", AJson {
        { "action", "scroll" },
        { "pointer", mPointer == Pointer::FINGER ? "finger" : "mouse" },
        { "x", target->x },
        { "y", target->y },
        { "dx", event.delta.x },
        { "dy", event.delta.y },
    });
    event.delta = { 0, 0 };   // consumed, the ruler area has nothing to scroll
}

void RemoteViewport::onKeyDown(AInput::Key key) {
    AView::onKeyDown(key);
    if (mStream) {
        mStream->call("input.key", AJson { { "key", keyName(key) }, { "action", "down" } });
    }
}

void RemoteViewport::onKeyUp(AInput::Key key) {
    AView::onKeyUp(key);
    if (mStream) {
        mStream->call("input.key", AJson { { "key", keyName(key) }, { "action", "up" } });
    }
}

void RemoteViewport::onCharEntered(AChar c) {
    AView::onCharEntered(c);
    if (!mStream || char32_t(c) < 0x20 || char32_t(c) == 0x7f) {
        return;   // control characters come as keys
    }
    mStream->call("input.type", AJson { { "text", AString(1, c) } });
}
