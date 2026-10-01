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

#include <atomic>
#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/AOptional.h>
#include <AUI/IO/APath.h>
#include <AUI/Platform/ASingleInstance.h>

class IEventLoop;

namespace aui::detail::application {
API_AUI_CORE int runMainLoopIfHeld(int entryExitCode);
}

/**
 * @brief Application-wide lifetime and instancing control.
 * @ingroup core
 * @details
 * See [app-lifetime] for the overview.
 *
 * `AApplication` is fully opt-in: if you never call its methods, your application behaves exactly as before (CLI
 * applications exit after `AUI_ENTRY`, UI applications exit after the last window is closed, embedded applications run
 * their own loop).
 *
 * ## Lifetime
 *
 * After `AUI_ENTRY` returns, AUI runs the main event loop as long as the application is *held*. Each shown `AWindow`
 * holds the application (unless `quitOnLastWindowClosed` is `false`). You can hold the application explicitly with
 * hold().
 *
 * ## Single instance
 *
 * See requestSingleInstanceLock().
 *
 * ## Data directory
 *
 * Application's own persistent data (settings, databases, etc.) should be stored in the per-application folder
 * returned by dataDir(). Unlike `APath::APPDATA`, which is shared between all applications of the user, it is derived
 * from the application id specified in `aui_app(ID ...)`. The folder is created by `dataDir()` on first call.
 *
 * ```cpp
 * APath settings = AApplication::inst().dataDir() / "settings.json";
 * ```
 */
class API_AUI_CORE AApplication : public AObject {
public:
    /**
     * @brief RAII object that keeps the application alive.
     * @details
     * While at least one Hold exists, the main event loop keeps running even if there are no open windows.
     */
    class API_AUI_CORE Hold : public aui::noncopyable {
    public:
        Hold();
        ~Hold();
    };

    static AApplication& inst();

    /**
     * @brief Creates a new application hold.
     * @details
     * Store the result in a long-living object (a controller, a tracker, a tray icon, the application state, etc.).
     * `AUI_ENTRY` returns before the main loop starts, so a local variable inside `AUI_ENTRY` is not enough.
     *
     * To stop holding (i.e., a window is closed and the user didn't allow background work), reset the pointer:
     * when the last hold is released, the application exits.
     *
     * ```cpp
     * state->lifetimeHold = AApplication::inst().hold();
     * ...
     * state->lifetimeHold.reset(); // allow the application to exit
     * ```
     */
    [[nodiscard]]
    _<Hold> hold();

    /**
     * @brief Number of active holds (explicit ones and implicit ones made by windows).
     */
    [[nodiscard]]
    int holdCount() const noexcept { return mHoldCount; }

    /**
     * @brief Whether the main loop should keep running.
     */
    [[nodiscard]]
    bool isHeld() const noexcept { return !mQuitRequested && mHoldCount > 0; }

    /**
     * @brief Requests the main event loop to stop regardless of holds.
     * @param exitCode if set, overrides the value returned from `AUI_ENTRY`.
     */
    void quit(AOptional<int> exitCode = std::nullopt);

    /**
     * @brief Whether quit() was called.
     */
    [[nodiscard]]
    bool isQuitRequested() const noexcept { return mQuitRequested; }

    /**
     * @brief Makes the current process the only running instance of the application.
     * @param key unique key; defaults to `aui::app_info::app_id` (set by `aui_app(ID ...)`).
     * @return true, if the current process is the primary instance; false, if another instance is running. In the
     * latter case, the activation (args, working dir, activation token) of the current process has already been
     * delivered to the primary instance and you should return from `AUI_ENTRY` immediately.
     * @details
     * Should be called at the very beginning of `AUI_ENTRY`, before any window is created. Code that may relaunch the
     * executable (i.e., an updater) should run before the lock.
     *
     * When another instance is launched, the primary instance receives activated signal on the main thread. For
     * applications working in background, create the window lazily in the handler and keep the application alive with
     * hold().
     *
     * ```cpp
     * AUI_ENTRY {
     *     if (!AApplication::inst().requestSingleInstanceLock()) {
     *         return 0;
     *     }
     *     auto state = _new<State>();
     *     state->lifetimeHold = AApplication::inst().hold();
     *     AObject::connect(AApplication::inst().activated, AObject::GENERIC_OBSERVER, [=](const AActivation& a) {
     *         if (!gMainWindow) {
     *             gMainWindow = _new<MainWindow>(state);
     *         }
     *         gMainWindow->activate(a.activationToken);
     *     });
     *     ...
     * }
     * ```
     *
     * See [app-lifetime] for the complete example.
     *
     * @specificto{android}
     * Always returns true.
     *
     * @specificto{ios}
     * Always returns true.
     *
     * @specificto{emscripten}
     * Always returns true.
     */
    bool requestSingleInstanceLock(AOptional<AString> key = std::nullopt);

    /**
     * @brief Whether requestSingleInstanceLock() succeeded.
     */
    [[nodiscard]]
    bool hasSingleInstanceLock() const noexcept { return mSingleInstance != nullptr; }

    /**
     * @brief Emitted on the main thread when another instance of the application was launched and redirected to this
     * one.
     * @details
     * Not emitted for the first launch: handle it in `AUI_ENTRY`.
     */
    emits<AActivation> activated;

    /**
     * @brief Per-application folder for persistent data (settings, databases, etc.).
     * @return absolute path to an existing (created on demand) folder.
     * @details
     * The folder name is derived from `aui::app_info::app_id` (set by `aui_app(ID ...)`). If the id is not set, the
     * `aui::app_info::name` is used instead. Different applications never share the same folder as long as their
     * ids (or names) differ.
     *
     * The folder is created by AUI: every call makes sure that it exists (including missing parent folders), so you
     * don't need to call `APath::makeDirs()` on it yourself. If the folder is removed while the application is
     * running (i.e., by the user), the next call to dataDir() creates it again; hence, don't cache the folder's
     * existence, call dataDir() when you need the path. Files and subfolders inside it are up to you.
     *
     * Throws AException if the application id (or name) is not set, or if the folder can't be created (i.e., no
     * permissions).
     *
     * ```cpp
     * APath settings = AApplication::inst().dataDir() / "settings.json";
     * ```
     *
     * @specificto{linux}
     * `$XDG_DATA_HOME/<app-id>` if `XDG_DATA_HOME` is set (flatpak, snap), otherwise `$HOME/.local/share/<app-id>`.
     *
     * @specificto{windows}
     * `%APPDATA%/<app-id>`.
     *
     * @specificto{android}
     * `<internal_storage_path>/__aui_appdata/<app-id>`.
     *
     * @specificto{ios}
     * `<internal_storage_path>/__aui_appdata/<app-id>`.
     *
     * See also [APath::getDefaultPath].
     */
    [[nodiscard]]
    APath dataDir() const;

    /**
     * @brief Whether a shown AWindow holds the application.
     * @details
     * Defaults to `true`, which means that the application exits when the last window is closed. Set to `false` to
     * manage the lifetime by hold() and quit() only.
     */
    std::atomic_bool quitOnLastWindowClosed = true;

private:
    friend class Hold;
    friend int aui::detail::application::runMainLoopIfHeld(int entryExitCode);
    AApplication();

    std::atomic_int mHoldCount = 0;
    std::atomic_bool mQuitRequested = false;
    AOptional<int> mExitCode;
    _unique<ASingleInstance> mSingleInstance;

    void onHoldReleased();
};

namespace aui::detail::application {
/**
 * @brief Called by aui_main after AUI_ENTRY returns.
 * @return exit code.
 */
API_AUI_CORE int runMainLoopIfHeld(int entryExitCode);

/**
 * @brief Event loop used by aui_main when the application is held but there is no other event loop (i.e., aui.views
 * is not used).
 */
API_AUI_CORE IEventLoop& fallbackEventLoop();
}
