/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/IO/IInputStream.h>
#include "Pipe.h"

class PipeInputStream: public IInputStream {
public:
    explicit PipeInputStream(Pipe pipe);
    ~PipeInputStream();

    size_t read(char* dst, size_t size) override;

private:
    Pipe mPipe;

#if AUI_PLATFORM_UNIX
    FILE* mFileHandle;
#endif
};
