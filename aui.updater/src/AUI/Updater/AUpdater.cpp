// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Curl/ACurl.h>
#include <AUI/IO/AByteBufferInputStream.h>
#include <AUI/Util/Archive.h>
#include "AUpdater.h"

bool AUpdater::needsExit(const AStringVector& applicationArguments) {
    return false;
}
void AUpdater::applyUpdateAndRestart() {}

void AUpdater::downloadAndUnpack(AString downloadUrl, const APath& unpackedUpdateDir) {
    aui::archive::zip::read(AByteBufferInputStream(ACurl::Builder(downloadUrl).runAsync()->body), aui::archive::ExtractTo{ unpackedUpdateDir });
}
