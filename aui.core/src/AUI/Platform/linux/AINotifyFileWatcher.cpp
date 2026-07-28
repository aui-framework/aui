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

#include "AINotifyFileWatcher.h"
#include "AUI/Platform/unix/UnixIoThread.h"
#include <unistd.h>
#include <sys/inotify.h>



AINotifyFileWatcher::AINotifyFileWatcher():
  mHandle(inotify_init1(IN_NONBLOCK))
{
    if (mHandle < 0) {
        throw AException("Failed to initialize inotify");
    }

    UnixIoThread::inst().registerCallback(mHandle, UnixPollEvent::IN, [this](ABitField<UnixPollEvent> triggeredFlags) {
        inotify_event event;
        Event fwEvent;
        for (;;) {
            if (read(mHandle, &event, sizeof(event)) != sizeof(event)) {
                return;
            }
            fwEvent.watchDescriptor = event.wd;
            fwEvent.mask = reinterpret_cast<Mask&>(event.mask);
            fwEvent.cookie = event.cookie;
            if (event.len > 0) {
                fwEvent.name.resize(event.len);
                if (read(mHandle, fwEvent.name.data(), event.len) != event.len) {
                    return;
                }
            }
            emit fired(fwEvent);
        }

    });
}

AINotifyFileWatcher::~AINotifyFileWatcher() {
    UnixIoThread::inst().unregisterCallback(mHandle);
    close(mHandle);
}

int AINotifyFileWatcher::addWatch(const AString& path, ABitField<Mask> mask) {
    int wd = inotify_add_watch(mHandle, path.c_str(), reinterpret_cast<uint32_t&>(mask));
    if (wd < 0) {
        throw AException("Failed to add inotify watch on {}"_format(path.toStdString()));
    }
    return wd;
}

void AINotifyFileWatcher::removeWatch(int watchDescriptor) {
    if (inotify_rm_watch(mHandle, watchDescriptor) < 0)
        throw AException("Failed to remove inotify watch");
}
