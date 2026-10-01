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

#include <AUI/Platform/ASingleInstance.h>

#include <optional>

/**
 * @brief GApplication (D-Bus, `org.freedesktop.Application`) backend of ASingleInstance for Linux.
 * @details
 * Thin wrapper over GApplication (`HANDLES_COMMAND_LINE | SEND_ENVIRONMENT`) loaded dynamically from `libgio-2.0.so.0`.
 * GApplication owns the session bus name equal to the application id, exports `org.freedesktop.Application` and
 * forwards command line, working directory and environment (`XDG_ACTIVATION_TOKEN`) of secondary instances to the
 * primary one. Works across sandboxes (Flatpak, Snap). GTK is not required.
 *
 * Unavailable (Outcome::available == false) if GIO can't be loaded, the key is not a valid application id or the
 * session bus is not reachable. In this case ASingleInstance falls back to flock + unix socket.
 */
namespace aui::detail::single_instance::dbus {

struct Handle {
    virtual ~Handle() = default;
};

struct Outcome {
    /**
     * @brief false if the D-Bus backend can't be used at all.
     */
    bool available = false;

    /**
     * @brief non-null if the current process is the primary instance. If `available` but `primary` is null, then
     * another instance is running and activation has been forwarded to it.
     */
    _unique<Handle> primary;
};

/**
 * @return whether the key is valid D-Bus well-known name (and, thus, can be used with this backend).
 */
bool isValidBusName(const AString& key);

Outcome tryAcquire(const AString& key, ASingleInstance::Callback callback);

}   // namespace aui::detail::single_instance::dbus
