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

#include "UiThread.h"
#include "ViewJson.h"

namespace aui::remote {

namespace {
using view_json::describe;

AJson array(aui::impl::JsonArray items = {}) { return AJson(std::move(items)); }

AString lowercased(AString s) { return s.lowercase(); }

int intParam(const AJson& params, const char* name, int fallback) {
    if (auto v = params.containsOpt(name)) {
        return v->asInt();
    }
    return fallback;
}

bool listContains(const AJson& params, const char* name, const char* item) {
    if (auto v = params.containsOpt(name)) {
        for (const auto& i : v->asArray()) {
            if (i.asString() == item) {
                return true;
            }
        }
    }
    return false;
}

_<AView> viewFromParams(const AJson& params, RefTable& refs, const char* name = "ref") {
    if (auto ref = params.containsOpt(name)) {
        return refs.resolve(ref->asString());
    }
    throw RpcException(rpc_code::INVALID_PARAMS, AString("missing param: ") + name);
}
}   // namespace

_<AWindow> windowFromParams(const AJson& params, RefTable& refs) {
    if (auto id = params.containsOpt("windowId")) {
        auto view = refs.resolve(id->asString());
        if (auto window = _cast<AWindow>(view)) {
            return window;
        }
        throw RpcException(rpc_code::NO_SUCH_WINDOW, "windowId is not a window: " + id->asString(),
                           "use an id returned by window.list");
    }
    for (const auto& window : ASurface::getWindowManager().getWindows()) {
        if (!window->isPopup()) {
            return window;
        }
    }
    throw RpcException(rpc_code::NO_SUCH_WINDOW, "the application has no windows", "wait until a window is shown");
}

AJson findViews(RefTable& refs, const AJson& params) {
    refs.newGeneration();
    const auto root = params.contains("ref") ? viewFromParams(params, refs) : _<AView>(windowFromParams(params, refs));
    const auto type = lowercased(params.containsOpt("type") ? params["type"].asString() : AString {});
    const auto text = lowercased(params.containsOpt("text") ? params["text"].asString() : AString {});
    const auto assName = params.containsOpt("assName") ? params["assName"].asString() : AString {};
    const bool onlyInteractive = params.containsOpt("interactive") && params["interactive"].asBool();
    const bool includeInvisible = params.containsOpt("includeInvisible") && params["includeInvisible"].asBool();

    aui::impl::JsonArray found;
    view_json::visit(root, [&](const _<AView>& view) {
        if (!includeInvisible && view->getVisibility() != Visibility::VISIBLE) {
            return false;   // the whole branch is invisible
        }
        if (!type.empty() && !lowercased(view_json::typeName(view)).contains(type)) return true;
        if (!text.empty()) {
            // a button is found by its label, which is the text of a view inside of it
            const auto shown = view_json::interactiveRole(view).empty() ? view_json::textOf(view) : view_json::labelOf(view);
            if (!lowercased(shown).contains(text)) return true;
        }
        if (!assName.empty() && !view->getAssNames().contains(assName)) return true;
        if (onlyInteractive && view_json::interactiveRole(view).empty()) return true;
        found.push_back(describe(refs, view));
        return true;
    });
    return array(std::move(found));
}

void registerInspectionMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    dispatcher.registerMethod("window.list", JsonRpcDispatcher::Handler([&refs](const AJson&) {
        return onUiThread([&refs] {
            refs.newGeneration();
            aui::impl::JsonArray windows;
            for (const auto& window : ASurface::getWindowManager().getWindows()) {
                windows.push_back(AJson {
                  { "windowId", refs.add(window) },
                  { "title", *window->title() },
                  { "popup", window->isPopup() },
                  { "focused", window->isFocused() },
                  { "width", window->getSize().x },
                  { "height", window->getSize().y },
                  { "x", window->getWindowPosition().x },
                  { "y", window->getWindowPosition().y },
                  { "dpi", window->getDpiRatio() },
                  { "scalingFactor", window->getScalingParams().scalingFactor },
                });
            }
            return array(std::move(windows));
        });
    }));

    dispatcher.registerMethod("view.tree", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            refs.newGeneration();
            _<AView> root = params.contains("ref") ? viewFromParams(params, refs) : _<AView>(windowFromParams(params, refs));
            const auto depth = std::clamp(intParam(params, "depth", 8), 0, 64);
            return view_json::tree(refs, root, depth, listContains(params, "include", "inspector"));
        });
    }));

    dispatcher.registerMethod("view.properties", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            auto view = viewFromParams(params, refs);
            auto node = describe(refs, view);
            node["inspector"] = view_json::inspector(view);
            if (auto container = _cast<AViewContainerBase>(view)) {
                node["childCount"] = int(container->getViews().size());
            }
            return node;
        });
    }));

    dispatcher.registerMethod("view.find", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] { return findViews(refs, params); });
    }));

    dispatcher.registerMethod("view.hitTest", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            refs.newGeneration();
            auto window = windowFromParams(params, refs);
            const glm::ivec2 position { params["x"].asInt(), params["y"].asInt() };
            aui::impl::JsonArray path;
            if (auto hit = window->getViewAtRecursive(position)) {
                for (const auto& view : view_json::pathTo(window, hit.get())) {
                    path.push_back(AJson {
                      { "ref", refs.add(view) },
                      { "type", view_json::typeName(view) },
                    });
                }
            }
            return AJson { { "path", array(std::move(path)) } };
        });
    }));

    dispatcher.registerMethod("view.interactive", JsonRpcDispatcher::Handler([&refs](const AJson& params) {
        return onUiThread([&refs, params = params] {
            refs.newGeneration();
            auto window = windowFromParams(params, refs);
            aui::impl::JsonArray elements;
            view_json::visit(window, [&](const _<AView>& view) {
                if (view->getVisibility() != Visibility::VISIBLE) {
                    return false;
                }
                if (auto role = view_json::interactiveRole(view); !role.empty()) {
                    AJson element {
                      { "ref", refs.add(view) },
                      { "role", role },
                      { "label", view_json::labelOf(view) },
                      { "rect", view_json::geometry(view) },
                      { "actions", view_json::actionsOf(role) },
                    };
                    if (view->hasFocus()) {
                        element["focused"] = true;
                    }
                    elements.push_back(std::move(element));
                }
                return true;
            });
            return array(std::move(elements));
        });
    }));
}

}   // namespace aui::remote
