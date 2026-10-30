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
#include <atomic>
#include <chrono>

#include "AUI/Image/AImage.h"
#include "AUI/Image/png/PngImageLoader.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/APath.h"
#include "AUI/Platform/ASurface.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/ARaiiHelper.h"
#include "AUI/View/AScrollArea.h"
#include "Marks.h"
#include "RemoteOverlay.h"
#include "ViewJson.h"

namespace aui::remote {

namespace {

APath shotsFolder() {
    auto folder = APath::getDefaultPath(APath::TEMP) / "aui-remote";
    folder.makeDirs();
    return folder;
}

APath nextShotPath() {
    static std::atomic<unsigned> counter = 0;
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    return shotsFolder() / ("shot-" + AString::number(static_cast<long long>(ms)) + "-" +
                            AString::number(++counter) + ".png");
}

ARect<int> rectInWindow(const AView& view) {
    return ARect<int>::fromTopLeftPositionAndSize(view.getPositionInWindow(), view.getSize());
}

ARect<int> intersection(const ARect<int>& a, const ARect<int>& b) {
    return { glm::max(a.p1, b.p1), glm::min(a.p2, b.p2) };
}

bool isEmpty(const ARect<int>& r) { return r.p2.x <= r.p1.x || r.p2.y <= r.p1.y; }

/// The part of the view that is on the screen: inside the window and inside the scroll areas around it.
ARect<int> visiblePart(const AWindow& window, const _<AView>& view) {
    auto rect = intersection(rectInWindow(*view), ARect<int>::fromTopLeftPositionAndSize({}, window.getSize()));
    for (auto parent = view->getParent(); parent; parent = parent->getParent()) {
        if (auto area = dynamic_cast<AScrollArea*>(parent)) {
            rect = intersection(rect, rectInWindow(*area));
        }
    }
    return rect;
}

struct Candidate {
    _<AView> view;
    AString role;
    ARect<int> rect;   // logical pixels of the window
};

/// Interactive views that can be seen, in the order of the tree. A view inside a view that is already marked (the label
/// of a button) is left out: its mark would cover the outer one and click the same thing.
AVector<Candidate> interactiveCandidates(const _<AWindow>& window) {
    AVector<Candidate> result;
    view_json::visit(window, [&](const _<AView>& view) {
        if (view->getVisibility() != Visibility::VISIBLE) {
            return false;
        }
        const bool insideMarked = std::any_of(result.begin(), result.end(), [&](const Candidate& marked) {
            for (auto parent = view->getParent(); parent; parent = parent->getParent()) {
                if (parent == marked.view.get()) {
                    return true;
                }
            }
            return false;
        });
        if (insideMarked) {
            return true;
        }
        if (auto role = view_json::interactiveRole(view); !role.empty()) {
            if (auto rect = visiblePart(*window, view); !isEmpty(rect)) {
                result << Candidate { view, std::move(role), rect };
            }
        }
        return true;
    });
    return result;
}

}   // namespace

void registerScreenshotMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: windowId?, ref? (crop to the view), padding? (logical pixels around the view, default 0),
    //         annotate? (numbered boxes over the interactive views, listed in `marks`)
    // result: { path, width, height, scale, crop?, marks? }
    dispatcher.registerMethod("screenshot", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return AThread::main()->async([&refs, params = params] {
            auto window = windowFromParams(params, refs);

            // the remote control overlay must not appear in screenshots: render a clean frame to capture.
            RemoteOverlay::setHidden(*window, true);
            AUI_DEFER {
                RemoteOverlay::setHidden(*window, false);
                window->flagRedraw();
            };
            if (RemoteOverlay::isEnabled(*window)) {
                window->redraw();
            }

            AImage image;
            try {
                image = window->getRenderingContext()->makeScreenshot();
            } catch (const AException& e) {
                throw RpcException(rpc_code::NOT_SUPPORTED, "screenshot failed: " + e.getMessage(),
                                   "the rendering context of this platform can't capture the window");
            }
            if (image.width() == 0 || image.height() == 0) {
                throw RpcException(rpc_code::NOT_SUPPORTED, "the rendering context returned an empty screenshot",
                                   "screenshots are not implemented for this platform");
            }

            AJson result;
            // the screenshot is in device pixels; geometry everywhere else is in logical ones.
            const auto windowSize = glm::max(window->getSize(), glm::ivec2(1));
            const auto scale = float(image.width()) / float(windowSize.x);
            result["scale"] = scale;

            glm::ivec2 cropOrigin { 0, 0 };   // logical pixels
            if (auto ref = params.containsOpt("ref")) {
                auto view = refs.resolve(ref->asString());
                const int padding = params.containsOpt("padding") ? params["padding"].asInt() : 0;
                const auto topLeft = glm::max(view->getPositionInWindow() - glm::ivec2(padding), glm::ivec2(0));
                const auto bottomRight = glm::min(
                    view->getPositionInWindow() + view->getSize() + glm::ivec2(padding), windowSize);
                const auto size = bottomRight - topLeft;
                if (size.x <= 0 || size.y <= 0) {
                    throw RpcException(rpc_code::INVALID_PARAMS, "the view is outside of the window",
                                       "scroll it into view or check view.properties");
                }
                const glm::uvec2 pixelPosition(glm::vec2(topLeft) * scale);
                glm::uvec2 pixelSize(glm::vec2(size) * scale);
                pixelSize = glm::min(pixelSize, image.size() - pixelPosition);
                image = image.cropped(pixelPosition, pixelSize);
                cropOrigin = topLeft;
                result["crop"] = AJson {
                    { "x", topLeft.x },
                    { "y", topLeft.y },
                    { "width", size.x },
                    { "height", size.y },
                };
            }

            if (params.containsOpt("annotate") && params["annotate"].asBool()) {
                // the refs of the marks are of a new generation, valid right after this call
                refs.newGeneration();
                const auto imageRect = ARect<int>::fromTopLeftPositionAndSize({}, glm::ivec2(image.size()));
                AVector<marks::Mark> drawn;
                aui::impl::JsonArray listed;
                for (auto& candidate : interactiveCandidates(window)) {
                    const ARect<int> pixels {
                        glm::ivec2(glm::floor(glm::vec2(candidate.rect.p1 - cropOrigin) * scale)),
                        glm::ivec2(glm::ceil(glm::vec2(candidate.rect.p2 - cropOrigin) * scale)),
                    };
                    const auto onImage = intersection(pixels, imageRect);
                    if (isEmpty(onImage)) {
                        continue;
                    }
                    const int label = int(drawn.size()) + 1;
                    drawn << marks::Mark { label, onImage };
                    const auto size = candidate.rect.size();
                    listed.push_back(AJson {
                      { "label", label },
                      { "ref", refs.add(candidate.view) },
                      { "role", candidate.role },
                      { "text", view_json::labelOf(candidate.view) },
                      { "rect", AJson { { "x", candidate.rect.p1.x }, { "y", candidate.rect.p1.y },
                                        { "w", size.x }, { "h", size.y } } },
                    });
                }
                marks::draw(image, drawn, scale * window->getDpiRatio());
                result["marks"] = AJson(std::move(listed));
            }

            const auto path = nextShotPath();
            PngImageLoader::save(AFileOutputStream(path), image);
            result["path"] = path.toStdString();
            result["width"] = int(image.width());
            result["height"] = int(image.height());
            return result;
        });
    }));
}

}   // namespace aui::remote
