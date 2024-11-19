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

#include "ABuiltinFiles.h"

#include "LZ.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/AByteBufferInputStream.h"

_unique<IInputStream> ABuiltinFiles::open(const AString& file) {
    if (auto c = inst().mBuffers.contains(file.toStdString())) {
        return aui::zlib::decompressToStream(c->second);
    }
    return nullptr;
}

ABuiltinFiles& ABuiltinFiles::inst() {
    static ABuiltinFiles f;
    return f;
}

void ABuiltinFiles::registerAsset(std::string_view path, const unsigned char* data, size_t size,
                                  std::string_view programModule) {
    inst().mBuffers[path] = AByteBufferView(reinterpret_cast<const char*>(data), size);
}

bool ABuiltinFiles::contains(const AString& file) {
    return inst().mBuffers.contains(file.toStdString());
}
