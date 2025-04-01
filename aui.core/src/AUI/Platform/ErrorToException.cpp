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
// Created by Alex2772 on 5/18/2022.
//

#include <AUI/IO/AIOException.h>
#include <AUI/Platform/ErrorToException.h>


void aui::impl::unix_based::lastErrorToException(AString message) {
    message += ": ";
    message += formatSystemError().description;
    switch (errno) {
        case ENOENT:
            throw AFileNotFoundException(message);
        case EPERM:
        case EACCES:
            throw AAccessDeniedException(message);
        case EEXIST:
            break;
        case ENOSPC:
            throw ANoSpaceLeftException(message);
        default:
            throw AIOException(message);
    }
}

aui::impl::Error aui::impl::unix_based::formatSystemError() {
    return { errno, strerror(errno) };
}