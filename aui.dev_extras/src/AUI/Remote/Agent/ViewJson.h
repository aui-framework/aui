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

#pragma once

#include <functional>

#include "AUI/Common/AOptional.h"
#include "AUI/Json/AJson.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/View/AView.h"
#include "RefTable.h"

namespace aui::remote {

/**
 * @brief What a view looks like to a remote client, and heuristics on top of the view tree. UI thread only.
 */
namespace view_json {

/**
 * @return direct children of the view; empty for views that are not containers
 */
const AVector<_<AView>>& childrenOf(const _<AView>& view);

/**
 * @return type name of the view, i.e. `AButton`
 */
AString typeName(const _<AView>& view);

/**
 * @return the text the view shows, if it has any
 */
AString textOf(const _<AView>& view);

/**
 * @return the text of the view, or of its first descendant that has any (i.e., the label of a button)
 */
AString labelOf(const _<AView>& view);

/**
 * @return role of the view if a user can interact with it, i.e. `button`, `textbox`; empty otherwise. This is a
 * heuristic: AUI has no accessibility tree.
 */
AString interactiveRole(const _<AView>& view);

/**
 * @return actions a client may perform on the view, i.e. `clickable`, `typeable`, `keyable`.
 */
AJson actionsOf(const AString& role);

/**
 * @return `{x, y, w, h}` relative to the parent plus `absX`, `absY` relative to the window.
 */
AJson geometry(const _<AView>& view);

/**
 * @return whether the view and all its ancestors are visible
 */
bool isVisible(const _<AView>& view);

/**
 * @brief The view with its properties: ref, type, geometry, state, text, ASS names.
 */
AJson describe(RefTable& refs, const _<AView>& view);

/**
 * @brief debugInspectorInfo as `{name: value}`.
 */
AJson inspector(const _<AView>& view);

/**
 * @brief The view with its descendants in `children`.
 * @param depth how many levels of descendants to include; 0 for the view only.
 * @param withInspector add debugInspectorInfo to every node.
 */
AJson tree(RefTable& refs, const _<AView>& view, int depth, bool withInspector);

/**
 * @brief Visits the view and its descendants, parents before children. Return false from the callback to skip
 * the children of the view.
 */
void visit(const _<AView>& root, const std::function<bool(const _<AView>&)>& callback);

/**
 * @return chain of views from `root` to `target` inclusive; empty if `target` is not under `root`.
 */
AVector<_<AView>> pathTo(const _<AView>& root, const AView* target);

}   // namespace view_json
}   // namespace aui::remote
