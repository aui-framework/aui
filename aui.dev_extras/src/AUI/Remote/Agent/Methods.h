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

#include "JsonRpc.h"
#include "RefTable.h"

namespace aui::remote {

/**
 * @brief Looks up a window by the optional `windowId` param; without it, the first window that is not a popup.
 * @throws RpcException with NO_SUCH_WINDOW or STALE_REF
 */
_<AWindow> windowFromParams(const AJson& params, RefTable& refs);

/**
 * `window.list`, `view.tree`, `view.find`, `view.properties`, `view.hitTest`, `view.interactive`.
 */
void registerInspectionMethods(JsonRpcDispatcher& dispatcher, RefTable& refs);

}   // namespace aui::remote
