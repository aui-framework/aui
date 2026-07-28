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

#include <AUI/Common/AString.h>
#include <AUI/Common/ASignal.h>
#include "AUI/Util/ABitField.h"

class API_AUI_CORE AINotifyFileWatcher : public AObject {
public:
    enum class Mask : uint32_t {
        ACCESS = 0x00000001,                 /* File was accessed.  */
        MODIFY = 0x00000002,                 /* File was modified.  */
        ATTRIB = 0x00000004,                 /* Metadata changed.  */
        CLOSE_WRITE = 0x00000008,            /* Writtable file was closed.  */
        CLOSE_NOWRITE = 0x00000010,          /* Unwrittable file closed.  */
        CLOSE = CLOSE_WRITE | CLOSE_NOWRITE, /* Close.  */
        OPEN = 0x00000020,                   /* File was opened.  */
        MOVED_FROM = 0x00000040,             /* File was moved from X.  */
        MOVED_TO = 0x00000080,               /* File was moved to Y.  */
        MOVE = MOVED_FROM | MOVED_TO,        /* Moves.  */
        CREATE = 0x00000100,                 /* Subfile was created.  */
        DELETE = 0x00000200,                 /* Subfile was deleted.  */
        DELETE_SELF = 0x00000400,            /* Self was deleted.  */
        MOVE_SELF = 0x00000800,              /* Self was moved.  */

        /* Events sent by the kernel.  */
        UNMOUNT = 0x00002000,    /* Backing fs was unmounted.  */
        Q_OVERFLOW = 0x00004000, /* Event queued overflowed.  */
        IGNORED = 0x00008000,    /* File was ignored.  */

        /* Special flags.  */
        ONLYDIR = 0x01000000,     /* Only watch the path if it is a directory.  */
        DONT_FOLLOW = 0x02000000, /* Do not follow a sym link.  */
        EXCL_UNLINK = 0x04000000, /* Exclude events on unlinked objects.  */
        MASK_CREATE = 0x10000000, /* Only create watches.  */
        MASK_ADD = 0x20000000,    /* Add to the mask of an already existing watch.  */
        ISDIR = 0x40000000,       /* Event occurred against dir.  */
        ONESHOT = 0x80000000,     /* Only send event once.  */

        /* All events which a program can wait on.  */
        ALL_EVENTS =
            ACCESS | MODIFY | ATTRIB | CLOSE_WRITE | CLOSE_NOWRITE | OPEN | MOVED_FROM | MOVED_TO | CREATE | DELETE |
            DELETE_SELF | MOVE_SELF
    };

    struct Event {
        int watchDescriptor;
        ABitField<Mask> mask;
        uint32_t cookie;
        AString name;
    };


    AINotifyFileWatcher();
    ~AINotifyFileWatcher();

    int addWatch(const AString& path, ABitField<Mask> mask);
    void removeWatch(int watchDescriptor);

    emits<Event> fired;

private:
    int mHandle;
};
