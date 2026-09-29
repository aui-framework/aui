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
#include <AUI/Common/AStringVector.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/IO/APath.h>
#include <AUI/Traits/values.h>

/**
 * @brief Describes an attempt to launch (activate) an application.
 * @ingroup core
 * @details
 * Delivered to the primary instance when another instance of the application is launched.
 */
struct AActivation {
    /**
     * @brief Command line arguments of the launched instance (including argv[0]).
     */
    AStringVector args;

    /**
     * @brief Working directory of the launched instance.
     */
    APath workingDir;

    /**
     * @brief Platform-specific token allowing the primary instance to steal focus.
     * @details
     * On Linux, `XDG_ACTIVATION_TOKEN` (Wayland) or `DESKTOP_STARTUP_ID` (X11) of the launched instance. Empty on other
     * platforms.
     */
    AString activationToken;
};

namespace aui::detail::single_instance {
API_AUI_CORE std::string serialize(const AActivation& activation);
API_AUI_CORE AActivation deserialize(std::string_view data);
API_AUI_CORE AActivation makeCurrentActivation();
API_AUI_CORE AString sanitizeKey(const AString& key);
}

/**
 * @brief Low-level cross-process single instance lock with activation forwarding.
 * @ingroup core
 * @details
 * Most users would prefer AApplication::requestSingleInstanceLock.
 *
 * | Platform | Implementation                                                             |
 * |----------|----------------------------------------------------------------------------|
 * | Windows  | Named mutex `Local\aui.<key>.<session>` + named pipe                       |
 * | Linux    | session D-Bus name `<key>` + `org.freedesktop.Application.Activate`, if available |
 * | Unix     | `flock` on `$XDG_RUNTIME_DIR/aui-<key>-<uid>.lock` + unix domain socket (fallback) |
 * | Other    | Always primary                                                             |
 *
 * The lock is released automatically by the OS if the process dies, so a crashed primary instance never blocks new
 * instances.
 */
class API_AUI_CORE ASingleInstance : public aui::noncopyable {
public:
    /**
     * @brief Activation callback. Called from a background thread.
     */
    using Callback = std::function<void(AActivation)>;

    /**
     * @brief Tries to become the primary instance for the given key.
     * @param key unique key of the application (i.e., app id).
     * @param onActivation called (from a background thread) when another instance forwards its activation.
     * @return lock object if the current process is the primary instance. nullptr if another instance is running; in
     * this case, activation of the current process has already been forwarded (synchronously) to the primary instance.
     */
    [[nodiscard]]
    static _unique<ASingleInstance> acquire(const AString& key, Callback onActivation);

    ~ASingleInstance();

    struct Impl;

    explicit ASingleInstance(_<Impl> impl): mImpl(std::move(impl)) {}

private:
    _<Impl> mImpl;
};
