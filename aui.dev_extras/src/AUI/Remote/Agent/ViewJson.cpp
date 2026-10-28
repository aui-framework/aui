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

#include "ViewJson.h"

#include "AUI/Common/IStringable.h"
#include "AUI/Reflect/AReflect.h"
#include "AUI/View/AAbstractTextField.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/View/ARadioButton.h"
#include "AUI/View/AViewContainer.h"

namespace aui::remote::view_json {

namespace {
AJson array(aui::impl::JsonArray items = {}) { return AJson(std::move(items)); }

}   // namespace

const AVector<_<AView>>& childrenOf(const _<AView>& view) {
    static const AVector<_<AView>> none;
    if (auto container = _cast<AViewContainerBase>(view)) {
        return container->getViews();
    }
    return none;
}

AString typeName(const _<AView>& view) { return AReflect::name(view.get()); }

AString textOf(const _<AView>& view) {
    if (auto stringable = dynamic_cast<const IStringable*>(view.get())) {
        return stringable->toString();
    }
    return {};
}

AString labelOf(const _<AView>& view) {
    AString label = textOf(view);
    if (!label.empty()) {
        return label;
    }
    visit(view, [&](const _<AView>& v) {
        if (!label.empty()) {
            return false;
        }
        label = textOf(v);
        return true;
    });
    return label;
}

AString interactiveRole(const _<AView>& view) {
    if (!isVisible(view) || !*view->enabled()) {
        return {};
    }
    if (dynamic_cast<ADropdownList*>(view.get())) return "dropdown";   // before AButton, it is derived from it
    if (dynamic_cast<AButton*>(view.get())) return "button";
    if (dynamic_cast<ACheckBox*>(view.get())) return "checkbox";
    if (dynamic_cast<ARadioButton*>(view.get())) return "radiobutton";
    if (dynamic_cast<AAbstractTextField*>(view.get())) return "textbox";
    if (view->capturesFocus() || view->handlesNonMouseNavigation()) return "focusable";
    if (view->clicked.hasOutgoingConnections() || view->clickedButton.hasOutgoingConnections()) return "clickable";
    return {};
}

AJson actionsOf(const AString& role) {
    if (role == "textbox") return array({ AJson("clickable"), AJson("typeable") });
    if (role == "focusable") return array({ AJson("clickable"), AJson("keyable") });
    return array({ AJson("clickable") });
}

AJson geometry(const _<AView>& view) {
    const auto position = view->getPosition();
    const auto size = view->getSize();
    const auto absolute = view->getPositionInWindow();
    return AJson {
        { "x", position.x },
        { "y", position.y },
        { "w", size.x },
        { "h", size.y },
        { "absX", absolute.x },
        { "absY", absolute.y },
    };
}

bool isVisible(const _<AView>& view) { return view->getVisibilityRecursive() == Visibility::VISIBLE; }

AJson describe(RefTable& refs, const _<AView>& view) {
    AJson node {
        { "ref", refs.add(view) },
        { "type", typeName(view) },
        { "geometry", geometry(view) },
        { "visible", isVisible(view) },
        { "enabled", bool(*view->enabled()) },
        { "focused", view->hasFocus() },
    };
    if (auto text = textOf(view); !text.empty()) {
        node["text"] = text;
    }
    aui::impl::JsonArray assNames;
    for (const auto& name : view->getAssNames()) {
        assNames.push_back(AJson(name));
    }
    node["assNames"] = array(std::move(assNames));
    if (auto role = interactiveRole(view); !role.empty()) {
        node["role"] = role;
        node["actions"] = actionsOf(role);
        if (auto label = labelOf(view); !label.empty()) {
            node["label"] = label;
        }
    }
    return node;
}

AJson inspector(const _<AView>& view) {
    AJson result = AJson::fromString("{}");
    for (const auto& [name, value] : view->debugInspectorInfo()) {
        result[name] = value.renderAsString();
    }
    return result;
}

AJson tree(RefTable& refs, const _<AView>& view, int depth, bool withInspector) {
    auto node = describe(refs, view);
    if (withInspector) {
        node["inspector"] = inspector(view);
    }
    if (depth > 0) {
        aui::impl::JsonArray children;
        for (const auto& child : childrenOf(view)) {
            children.push_back(tree(refs, child, depth - 1, withInspector));
        }
        node["children"] = array(std::move(children));
    }
    return node;
}

void visit(const _<AView>& root, const std::function<bool(const _<AView>&)>& callback) {
    if (!callback(root)) {
        return;
    }
    for (const auto& child : childrenOf(root)) {
        visit(child, callback);
    }
}

AVector<_<AView>> pathTo(const _<AView>& root, const AView* target) {
    if (root.get() == target) {
        return { root };
    }
    for (const auto& child : childrenOf(root)) {
        if (auto path = pathTo(child, target); !path.empty()) {
            path.insert(path.begin(), root);
            return path;
        }
    }
    return {};
}

}   // namespace aui::remote::view_json
