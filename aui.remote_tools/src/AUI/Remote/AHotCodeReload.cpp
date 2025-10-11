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

//
// Created by alex2772 on 10/6/25.
//

#include "AHotCodeReload.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Platform/AProcess.h"
#include "ObjectLoader.h"
#include "AUI/Platform/linux/AINotifyFileWatcher.h"

namespace {

static constexpr auto LOG_TAG = "Hot code reload";

}

struct AHotCodeReload::Priv {
    ObjectLoader loader = AProcess::self()->getPathToExecutable();
    _<AINotifyFileWatcher> watcher = _new<AINotifyFileWatcher>();
};

AHotCodeReload::AHotCodeReload() = default;
AHotCodeReload::~AHotCodeReload() = default;

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

void AHotCodeReload::addFile(AString path) {
    ALogger::info(LOG_TAG) << "Watching: " << path;
    int h = mPriv->watcher->addWatch(path, AINotifyFileWatcher::Mask::MODIFY);
    AObject::connect(mPriv->watcher->fired, [this, path = std::move(path), h](const AINotifyFileWatcher::Event& event) mutable {
        if (event.watchDescriptor != h) {
            return;
        }
        loadBinary(path);
        AThread::main()->enqueue([this, h, path = std::move(path)]() mutable {
            try {
                mPriv->watcher->removeWatch(h);
            } catch (...) {}
            addFile(std::move(path));
        });
    });
}

void AHotCodeReload::addFiles(AStringView paths) {
    for (auto path : paths.split(';')) {
        addFile(std::move(path));
    }
}
