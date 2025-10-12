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


#include "AUI/Remote/AHotCodeReload.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Platform/AProcess.h"

#if AUI_PLATFORM_LINUX
#include "AUI/Platform/linux/ELFObjectLoader.h"
#include "AUI/Platform/linux/AINotifyFileWatcher.h"
#endif

// Shared part =========================================================================================================

namespace {
static constexpr auto LOG_TAG = "Hot code reload";
}

AHotCodeReload::AHotCodeReload() = default;
AHotCodeReload::~AHotCodeReload() = default;

void AHotCodeReload::addFiles(AStringView paths) {
    for (auto path : paths.split(';')) {
        addFile(std::move(path));
    }
}

AHotCodeReload& AHotCodeReload::inst() {
    static auto instance = aui::ptr::manage_shared(new AHotCodeReload);
    return *instance;
}

#if AUI_PLATFORM_LINUX

struct AHotCodeReload::Priv {
    ELFObjectLoader loader = AProcess::self()->getPathToExecutable();
    _<AINotifyFileWatcher> watcher = _new<AINotifyFileWatcher>();
};

void AHotCodeReload::loadBinary(const APath& path) {
    AThread::main()->enqueue([this, self = shared_from_this(), path] {
      int attempts = 10;
        tryAgain:
        try {
            ALogger::info(LOG_TAG) << "Reloading: " << path;
            emit patchBegin;
            mPriv->loader.load(path);
            AUI_DEFER { emit patchEnd; };
            ALogger::info(LOG_TAG) << "Binary reloaded: " << path;
        } catch (const AException& e) {
            if (attempts-- > 0) {
                // there's a weird datarace
                AThread::sleep(std::chrono::milliseconds(100));
                goto tryAgain;
            }
            ALogger::err(LOG_TAG) << "Failed to reload binary: " << path << " : " << e;
        }
    });
}

void AHotCodeReload::addFile(AString path) noexcept {
    try {
        int h = mPriv->watcher->addWatch(path, AINotifyFileWatcher::Mask::MODIFY);
        ALogger::info(LOG_TAG) << "Watching: " << path;
        AObject::connect(
            mPriv->watcher->fired, [this, path = std::move(path), h](const AINotifyFileWatcher::Event& event) mutable {
                if (event.watchDescriptor != h) {
                    return;
                }
                loadBinary(path);
                AThread::main()->enqueue([this, h, path = std::move(path)]() mutable {
                    try {
                        mPriv->watcher->removeWatch(h);
                    } catch (...) {
                    }
                    addFile(std::move(path));
                });
            });
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Failed to add file to watch: " << path << " : " << e;
    }
}

#else

struct AHotCodeReload::Priv {};

void AHotCodeReload::addFile(AString path) {
    throw AException("Not implemented");
}

void AHotCodeReload::loadBinary(const APath& path) {
    throw AException("Not implemented");
}

#endif
