/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Methods.h"

#include <algorithm>
#include <numbers>

#include "AUI/Event/AGestureEvent.h"
#include "AUI/Event/AInjectedEvent.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/AWindow.h"
#include "RemoteOverlay.h"
#include "UiThread.h"

namespace aui::remote {

namespace {

AInput::Key keyByName(AString name) {
    name = name.uppercase();
    static const AMap<AString, AString> ALIASES = {
        { "ENTER", "RETURN" },    { "ESC", "ESCAPE" },       { "CTRL", "LCONTROL" }, { "CONTROL", "LCONTROL" },
        { "SHIFT", "LSHIFT" },    { "ALT", "LALT" },         { "CMD", "LSYSTEM" },   { "META", "LSYSTEM" },
        { "SUPER", "LSYSTEM" },   { "DELETE", "DEL" },
    };
    if (auto alias = ALIASES.contains(name)) {
        name = ALIASES.at(name);
    }
    const auto& names = AEnumerate<AInput::Key>::nameToValueMap();
    for (const auto& candidate : { name, "AInput::" + name }) {
        if (auto it = names.find(candidate); it != names.end()) {
            return it->second;
        }
    }
    throw RpcException(rpc_code::INVALID_PARAMS, "unknown key: " + name,
                       "use AInput::Key names, i.e. RETURN, ESCAPE, TAB, LEFT, F5, A, NUM1");
}

AInput::Key buttonByName(const AJson& params) {
    if (auto b = params.containsOpt("button")) {
        const auto name = b->asString().lowercase();
        if (name == "left") return AInput::LBUTTON;
        if (name == "right") return AInput::RBUTTON;
        if (name == "middle") return AInput::CBUTTON;
        throw RpcException(rpc_code::INVALID_PARAMS, "unknown button: " + name, "left, right or middle");
    }
    return AInput::LBUTTON;
}

AVector<AInput::Key> modifiersOf(const AJson& params) {
    AVector<AInput::Key> result;
    if (auto m = params.containsOpt("modifiers")) {
        for (const auto& name : m->asArray()) {
            result << keyByName(name.asString());
        }
    }
    return result;
}

/// Makes AInput::isKeyDown report the keys as pressed while the synthetic input is being delivered.
class OverrideScope {
public:
    OverrideScope(AVector<AInput::Key> keys) : mKeys(std::move(keys)) {
        for (auto k : mKeys) {
            AInput::overrideState(k, true);
        }
    }
    ~OverrideScope() {
        for (auto k : mKeys) {
            AInput::resetOverride(k);
        }
    }
    OverrideScope(const OverrideScope&) = delete;

private:
    AVector<AInput::Key> mKeys;
};

struct Target {
    _<AWindow> window;
    _<AView> view;   // may be null
    glm::vec2 position;
};

float number(const AJson& params, const char* name) {
    if (auto v = params.containsOpt(name)) {
        return v->isInt() ? float(v->asInt()) : float(v->asNumber());
    }
    throw RpcException(rpc_code::INVALID_PARAMS, AString("missing param: ") + name);
}

/// A point by `<prefix>ref` (center of the view) or `<prefix>x`/`<prefix>y` (window coordinates).
Target resolveTarget(const AJson& params, RefTable& refs, const char* prefix = "") {
    const AString refName = AString(prefix) + "ref";
    if (auto ref = params.containsOpt(refName.toStdString().c_str())) {
        auto view = refs.resolve(ref->asString());
        auto window = _cast<AWindow>(aui::ptr::shared_from_this(view->getWindow()));
        if (!window) {
            throw RpcException(rpc_code::NO_SUCH_WINDOW, "the view is not attached to a window",
                               "it may have been removed; re-run view.find");
        }
        return { window, view, glm::vec2(view->getCenterPointInWindow()) };
    }
    const AString x = AString(prefix) + "x", y = AString(prefix) + "y";
    return { windowFromParams(params, refs), nullptr,
             glm::vec2(number(params, x.toStdString().c_str()), number(params, y.toStdString().c_str())) };
}

AJson describePoint(const Target& target) {
    return AJson {
        { "x", int(target.position.x) },
        { "y", int(target.position.y) },
    };
}

/// Runs `first` on the UI thread, then `second` on the next iteration of its queue, so asynchronous effects of `first`
/// (i.e. AView::focus) are applied.
template <typename First, typename Second>
AFuture<AJson> inTwoSteps(First first, Second second) {
    return AFuture<AJson>::create([&](AFuture<AJson>::Resolver result) {
        AThread::main()->enqueue(
            [result = std::move(result), first = std::move(first), second = std::move(second)]() mutable {
                try {
                    first();
                } catch (...) {
                    result.supplyException();
                    return;
                }
                AThread::main()->enqueue([result = std::move(result), second = std::move(second)]() mutable {
                    try {
                        result.supplyValue(second());
                    } catch (...) {
                        result.supplyException();
                    }
                });
            });
    });
}

constexpr auto CLICK_DURATION = std::chrono::milliseconds(50);

// AUI has no long press timer of its own: on Android ALongPressEvent comes from the system GestureDetector, whose
// timeout is 400-500 ms. Holding a bit longer looks like a real long press there.
constexpr auto LONG_PRESS_DURATION = std::chrono::milliseconds(600);

// touch events of a 60 Hz screen. ATouchScroller takes the velocity from the distance between consecutive moves
// (times 60), so moves have to come at this pace for a fling to be as fast as asked.
constexpr auto TOUCH_FRAME = std::chrono::milliseconds(16);
constexpr float TOUCH_FRAMES_PER_SECOND = 60.f;

constexpr auto MAX_GESTURE_DURATION = std::chrono::seconds(30);

int intOr(const AJson& params, const char* name, int fallback) {
    if (auto v = params.containsOpt(name)) {
        return v->isInt() ? v->asInt() : int(v->asNumber());
    }
    return fallback;
}

float numberOr(const AJson& params, const char* name, float fallback) {
    return params.containsOpt(name) ? number(params, name) : fallback;
}

/// A finger moving through points; times are milliseconds since the start of the gesture.
struct Track {
    int finger = 0;
    AVector<std::pair<std::chrono::milliseconds, glm::vec2>> points;
};

/// Presses, moves and releases of all fingers, in the order of time.
struct TouchEvent {
    enum class Kind { PRESS, MOVE, RELEASE };
    std::chrono::milliseconds time;
    int finger;
    Kind kind;
    glm::vec2 position;
};

AVector<TouchEvent> timelineOf(const AVector<Track>& tracks) {
    AVector<TouchEvent> events;
    for (const auto& track : tracks) {
        const auto& points = track.points;
        events << TouchEvent { points.first().first, track.finger, TouchEvent::Kind::PRESS, points.first().second };
        for (std::size_t i = 1; i < points.size(); ++i) {
            const auto [fromTime, from] = points[i - 1];
            const auto [toTime, to] = points[i];
            // in between the points, the finger moves smoothly, a move per screen frame
            for (auto t = fromTime + TOUCH_FRAME; t < toTime; t += TOUCH_FRAME) {
                const auto progress = float((t - fromTime).count()) / float((toTime - fromTime).count());
                events << TouchEvent { t, track.finger, TouchEvent::Kind::MOVE, glm::mix(from, to, progress) };
            }
            events << TouchEvent { toTime, track.finger, TouchEvent::Kind::MOVE, to };
        }
        events << TouchEvent { points.last().first, track.finger, TouchEvent::Kind::RELEASE, points.last().second };
    }
    // at the same moment, all fingers move before any of them is lifted: a finger lifted first would turn the last
    // step of a two-finger gesture into a one-finger one
    std::stable_sort(events.begin(), events.end(),
                     [](const auto& a, const auto& b) { return std::tie(a.time, a.kind) < std::tie(b.time, b.kind); });
    return events;
}

/// Plays the fingers in real time; resumes on the UI thread when all of them are released.
AFuture<> playTracks(_<AWindow> window, AVector<Track> tracks) {
    const auto events = timelineOf(tracks);
    const auto start = std::chrono::steady_clock::now();
    RemoteOverlay::enable(*window);
    for (const auto& e : events) {
        const auto due = start + e.time;
        if (const auto now = std::chrono::steady_clock::now(); due > now) {
            co_await AThread::asyncSleep(std::chrono::duration_cast<std::chrono::milliseconds>(due - now));
        }
        const auto index = APointerIndex::finger(e.finger);
        switch (e.kind) {
            case TouchEvent::Kind::PRESS:
                window->injectEvent(APointerPressedEvent { e.position, index });
                break;
            case TouchEvent::Kind::MOVE:
                window->injectEvent(AInjectedPointerMove { e.position, APointerMoveEvent { index } });
                break;
            case TouchEvent::Kind::RELEASE:
                window->injectEvent(APointerReleasedEvent { e.position, index });
                break;
        }
        RemoteOverlay::showPointer(*window, e.position, index, e.kind == TouchEvent::Kind::PRESS,
                                   e.kind != TouchEvent::Kind::RELEASE);
    }
}

void validate(const AVector<Track>& tracks) {
    if (tracks.empty()) {
        throw RpcException(rpc_code::INVALID_PARAMS, "no fingers", "pass fingers: [{ finger, points: [{ x, y, t }] }]");
    }
    AVector<int> seen;
    for (const auto& track : tracks) {
        if (track.finger < 0 || track.finger > 9 || seen.contains(track.finger)) {
            throw RpcException(rpc_code::INVALID_PARAMS, "finger indices must be unique, 0..9");
        }
        seen << track.finger;
        if (track.points.empty()) {
            throw RpcException(rpc_code::INVALID_PARAMS, "a finger without points");
        }
        for (std::size_t i = 0; i < track.points.size(); ++i) {
            const auto t = track.points[i].first;
            if (t.count() < 0 || (i > 0 && t < track.points[i - 1].first) || t > MAX_GESTURE_DURATION) {
                throw RpcException(rpc_code::INVALID_PARAMS, "point times must grow from 0 and stay under 30000 ms");
            }
        }
    }
}

/// Two fingers placed symmetrically around `center`, moving for `duration`; `at(progress)` gives the distance from the
/// center to a finger and the angle of the line between the fingers.
template <typename At>
AVector<Track> twoFingers(glm::vec2 center, std::chrono::milliseconds duration, At at) {
    AVector<Track> tracks { Track { 0, {} }, Track { 1, {} } };
    const int steps = std::max(1, int(duration / TOUCH_FRAME));
    for (int i = 0; i <= steps; ++i) {
        const float progress = float(i) / float(steps);
        const auto [radius, angle] = at(progress);
        const glm::vec2 offset = glm::vec2(std::cos(angle), std::sin(angle)) * radius;
        const auto time = std::chrono::milliseconds(duration.count() * i / steps);
        tracks[0].points << std::pair { time, center - offset };
        tracks[1].points << std::pair { time, center + offset };
    }
    return tracks;
}

// params: fingers: [{ finger?, points: [{ x, y, t }] }] | gesture (pinch|rotate) with ref | x, y (the center),
//         pinch: from, to (distance between the fingers, px), angle? (degrees, 0 = horizontal);
//         rotate: degrees, distance? (between the fingers, default 100); durationMs? (default 300); windowId?
AFuture<AJson> inputTouch(RefTable& refs, AJson params) {
    AVector<Track> tracks;
    _<AWindow> window;
    if (auto fingers = params.containsOpt("fingers")) {
        window = windowFromParams(params, refs);
        int nextFinger = 0;
        for (const auto& f : fingers->asArray()) {
            Track track { intOr(f, "finger", nextFinger), {} };
            nextFinger = track.finger + 1;
            for (const auto& p : f["points"].asArray()) {
                track.points << std::pair { std::chrono::milliseconds(intOr(p, "t", 0)),
                                            glm::vec2(number(p, "x"), number(p, "y")) };
            }
            tracks << std::move(track);
        }
    } else if (auto gesture = params.containsOpt("gesture")) {
        auto target = resolveTarget(params, refs);
        window = target.window;
        const auto duration = std::chrono::milliseconds(std::clamp(intOr(params, "durationMs", 300), 16, 30000));
        constexpr float DEG = std::numbers::pi_v<float> / 180.f;
        if (gesture->asString() == "pinch") {
            const float from = number(params, "from") / 2.f, to = number(params, "to") / 2.f;
            const float angle = numberOr(params, "angle", 0.f) * DEG;
            tracks = twoFingers(target.position, duration, [&](float t) { return std::pair { glm::mix(from, to, t), angle }; });
        } else if (gesture->asString() == "rotate") {
            const float radius = numberOr(params, "distance", 100.f) / 2.f;
            const float degrees = number(params, "degrees") * DEG;
            tracks = twoFingers(target.position, duration, [&](float t) { return std::pair { radius, degrees * t }; });
        } else {
            throw RpcException(rpc_code::INVALID_PARAMS, "unknown gesture: " + gesture->asString(), "pinch or rotate");
        }
    } else {
        throw RpcException(rpc_code::INVALID_PARAMS, "pass fingers or gesture",
                           "fingers: [{ finger, points: [{ x, y, t }] }], or gesture: pinch | rotate");
    }
    validate(tracks);
    co_await playTracks(window, tracks);
    std::chrono::milliseconds duration { 0 };
    for (const auto& track : tracks) {
        duration = std::max(duration, track.points.last().first);
    }
    co_return AJson {
        { "fingers", int(tracks.size()) },
        { "durationMs", int(duration.count()) },
    };
}

// params: action (click|doubleClick|press|release|move|drag|scroll|longPress|fling; default click), ref | x, y,
//         pointer (mouse|finger; default: defaultPointer()), button (left|right|middle; mouse only), modifiers, drag: toRef | toX, toY, scroll: dx, dy,
//         longPress: durationMs; fling: dx, dy (the way of the finger), velocity (px/s, default 2000).
//         longPress and fling are touch gestures, they use a finger whatever `pointer` says.
// A coroutine, so `click` can hold the button without blocking the UI thread. Resumes on the UI thread.
AFuture<AJson> inputPointer(RefTable& refs, AJson params) {
    const auto action = params.containsOpt("action") ? params["action"].asString() : AString("click");
    const auto pointer = params.containsOpt("pointer") ? params["pointer"].asString() : defaultPointer();
    if (pointer != "mouse" && pointer != "finger") {
        throw RpcException(rpc_code::INVALID_PARAMS, "unknown pointer: " + pointer, "mouse or finger");
    }
    const bool finger = pointer == "finger";
    const auto button = buttonByName(params);
    const auto index = finger ? APointerIndex::finger(0) : APointerIndex::button(button);
    auto target = resolveTarget(params, refs);
    auto& window = target.window;
    auto position = target.position;

    OverrideScope modifiers(modifiersOf(params));
    // a finger has no buttons for AInput to report
    auto overrideButton = [&](bool down) {
        if (finger) {
            return;
        }
        if (down) {
            AInput::overrideState(button, true);
        } else {
            AInput::resetOverride(button);
        }
    };
    auto move = [&](glm::vec2 to) {
        window->injectEvent(AInjectedPointerMove { to, APointerMoveEvent { index } });
    };
    auto press = [&] {
        overrideButton(true);
        window->injectEvent(APointerPressedEvent { position, index });
    };
    auto release = [&] {
        window->injectEvent(APointerReleasedEvent { position, index });
        overrideButton(false);
    };

    if (action == "move") {
        move(position);
    } else if (action == "press") {
        // the button stays "down" for AInput until `release`; views read it via the event, so don't leak it.
        move(position);
        press();
        overrideButton(false);
    } else if (action == "release") {
        window->injectEvent(APointerReleasedEvent { position, index });
    } else if (action == "click") {
        move(position);
        press();
        // a real click lasts a moment; the caller gets the answer only after the button is released
        RemoteOverlay::showPointer(*window, position, index, true, true);
        co_await AThread::asyncSleep(CLICK_DURATION);
        release();
    } else if (action == "doubleClick") {
        move(position);
        press();
        release();
        overrideButton(true);
        window->injectEvent(AInjectedPointerDoubleClick { APointerPressedEvent { position, index } });
        release();
    } else if (action == "drag") {
        const auto to = resolveTarget(params, refs, "to").position;
        constexpr int STEPS = 10;
        move(position);
        press();
        overrideButton(true);
        for (int i = 1; i <= STEPS; ++i) {
            move(glm::mix(position, to, float(i) / STEPS));
        }
        position = to;
        release();
    } else if (action == "longPress") {
        const auto duration = std::chrono::milliseconds(intOr(params, "durationMs", LONG_PRESS_DURATION.count()));
        const auto finger = APointerIndex::finger(0);
        RemoteOverlay::enable(*window);
        window->injectEvent(APointerPressedEvent { position, finger });
        RemoteOverlay::showPointer(*window, position, finger, true, true);
        co_await AThread::asyncSleep(std::clamp(duration, std::chrono::milliseconds(0), std::chrono::milliseconds(30000)));
        // as the platform does when the timeout passes; a handled long press is not followed by a click
        const bool handled = window->onGesture(glm::ivec2(position), ALongPressEvent {});
        window->injectEvent(APointerReleasedEvent { position, finger });
        RemoteOverlay::showPointer(*window, position, finger, false, false);
        co_return AJson {
            { "at", describePoint(target) },
            { "handled", handled },
        };
    } else if (action == "fling") {
        const glm::vec2 way { numberOr(params, "dx", 0.f), numberOr(params, "dy", 0.f) };
        const float length = glm::length(way);
        if (length < 1.f) {
            throw RpcException(rpc_code::INVALID_PARAMS, "fling needs dx or dy", "the way of the finger in px");
        }
        const float velocity = std::max(1.f, numberOr(params, "velocity", 2000.f));
        // the velocity is the step between moves times 60; the last three steps count, so there are at least three
        const int steps = std::max(3, int(std::ceil(length / (velocity / TOUCH_FRAMES_PER_SECOND))));
        Track track { 0, {} };
        for (int i = 0; i <= steps; ++i) {
            track.points << std::pair { TOUCH_FRAME * i, position + way * (float(i) / float(steps)) };
        }
        co_await playTracks(window, { std::move(track) });
        co_return AJson {
            { "at", describePoint(target) },
            { "velocity", int(length / float(steps) * TOUCH_FRAMES_PER_SECOND) },
        };
    } else if (action == "scroll") {
        move(position);
        window->injectEvent(AScrollEvent {
          .origin = position,
          .delta = { params.containsOpt("dx") ? number(params, "dx") : 0.f,
                     params.containsOpt("dy") ? number(params, "dy") : 0.f },
        });
    } else {
        throw RpcException(rpc_code::INVALID_PARAMS, "unknown action: " + action,
                           "click, doubleClick, press, release, move, drag, scroll, longPress or fling");
    }
    RemoteOverlay::enable(*window);
    // between `press` and `release` the pointer stays on the screen
    RemoteOverlay::showPointer(*window, position, index, action != "move" && action != "scroll", action == "press");
    co_return AJson {
        { "at", describePoint(target) },
    };
}

/// Runs a coroutine of an input method on the UI thread.
template <typename Coroutine>
AFuture<AJson> onUiThreadAsync(RefTable& refs, AJson params, Coroutine coroutine) {
    return AFuture<AJson>::create([&](AFuture<AJson>::Resolver result) {
        AThread::main()->enqueue([&refs, params = std::move(params), result = std::move(result), coroutine] {
            try {
                auto work = coroutine(refs, params);
                result.onCancel([work] {});   // the coroutine runs while somebody waits for it
                work.onValue([result](const AJson& value) { result.supplyValue(value); });
                work.onException([result](const AException& e) {
                    result.supplyException(aui::impl::future::causeOf(e));
                });
            } catch (...) {
                result.supplyException();
            }
        });
    });
}

}   // namespace

AString defaultPointer() {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
    return "finger";
#else
    return "mouse";
#endif
}

void registerInputMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    dispatcher.registerMethod("input.pointer", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThreadAsync(refs, params, inputPointer);
    }));

    dispatcher.registerMethod("input.touch", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThreadAsync(refs, params, inputTouch);
    }));

    // params: gesture (longPress), ref | x, y, windowId?
    // Only the gesture, as the platform reports it; a client that holds a finger itself (the GUI) sends it when its
    // own long press timeout passes.
    dispatcher.registerMethod("input.gesture", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            const auto gesture = params.containsOpt("gesture") ? params["gesture"].asString() : AString {};
            if (gesture != "longPress") {
                throw RpcException(rpc_code::INVALID_PARAMS, "unknown gesture: " + gesture, "longPress");
            }
            auto target = resolveTarget(params, refs);
            RemoteOverlay::enable(*target.window);
            const bool handled = target.window->onGesture(glm::ivec2(target.position), ALongPressEvent {});
            return AJson {
                { "at", describePoint(target) },
                { "handled", handled },
            };
        });
    }));

    // params: text, ref? (focuses the view first)
    dispatcher.registerMethod("input.type", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        if (!params.containsOpt("text")) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: text");
        }
        const auto text = params["text"].asString();
        _<AView> view;
        if (auto ref = params.containsOpt("ref")) {
            view = refs.resolve(ref->asString());
        }
        const auto windowParams = params;
        return inTwoSteps(
            [view] {
                if (view) {
                    view->focus();
                }
            },
            [&refs, text, windowParams] {
                auto window = windowFromParams(windowParams, refs);
                int count = 0;
                for (AChar c : text) {
                    window->injectEvent(AInjectedChar { c });
                    ++count;
                }
                return AJson { { "typed", count } };
            });
    }));

    // params: key, modifiers?, ref? (focuses the view first), action? (press (default)|down|up)
    dispatcher.registerMethod("input.key", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        if (!params.containsOpt("key")) {
            throw RpcException(rpc_code::INVALID_PARAMS, "missing param: key");
        }
        const auto key = keyByName(params["key"].asString());
        const auto modifiers = modifiersOf(params);
        const auto action = params.containsOpt("action") ? params["action"].asString() : AString("press");
        _<AView> view;
        if (auto ref = params.containsOpt("ref")) {
            view = refs.resolve(ref->asString());
        }
        const auto windowParams = params;
        return inTwoSteps(
            [view] {
                if (view) {
                    view->focus();
                }
            },
            [&refs, key, modifiers, action, windowParams] {
                auto window = windowFromParams(windowParams, refs);
                OverrideScope scope(modifiers);
                if (action == "press" || action == "down") {
                    for (auto m : modifiers) {
                        window->injectEvent(AInjectedKey { AInjectedKey::Action::DOWN, m });
                    }
                    window->injectEvent(AInjectedKey { AInjectedKey::Action::DOWN, key });
                }
                if (action == "press" || action == "up") {
                    window->injectEvent(AInjectedKey { AInjectedKey::Action::UP, key });
                    for (auto m : modifiers) {
                        window->injectEvent(AInjectedKey { AInjectedKey::Action::UP, m });
                    }
                }
                return AJson { { "key", AEnumerate<AInput::Key>::toName(key) } };
            });
    }));
}

}   // namespace aui::remote
