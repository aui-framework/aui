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
#include "RemoteOverlay.h"
#include "UiThread.h"

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

}   // namespace

void registerScreenshotMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    // params: windowId?, ref? (crop to the view), padding? (logical pixels around the view, default 0)
    // result: { path, width, height, scale, crop? }
    dispatcher.registerMethod("screenshot", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
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
                result["crop"] = AJson {
                    { "x", topLeft.x },
                    { "y", topLeft.y },
                    { "width", size.x },
                    { "height", size.y },
                };
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
