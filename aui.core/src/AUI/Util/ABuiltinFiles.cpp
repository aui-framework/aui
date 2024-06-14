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

void ABuiltinFiles::loadBuffer(AByteBuffer& data)
{
	AByteBuffer unpacked;
	LZ::decompress(data, unpacked);
    AByteBufferInputStream bis(unpacked);
	while (bis.available())
	{
		std::string file;
        AByteBuffer b;
        bis >> aui::serialize_sized(file);
        bis >> aui::serialize_sized(b);
        inst().mBuffers[AString(file)] = std::move(b);
	}
}

_<IInputStream> ABuiltinFiles::open(const AString& file)
{
	if (auto c = inst().mBuffers.contains(file))
	{
		return _new<AByteBufferInputStream>(c->second);
	}
	return nullptr;
}

AOptional<AByteBufferView> ABuiltinFiles::getBuffer(const AString& file) {
    if (auto c = inst().mBuffers.contains(file))
    {
        return AByteBufferView(c->second);
    }
    return std::nullopt;
}

ABuiltinFiles& ABuiltinFiles::inst() {
    static ABuiltinFiles f;
    return f;
}

void ABuiltinFiles::load(const unsigned char* data, size_t size) {
    AByteBuffer b(data, size);
    inst().loadBuffer(b);
}

bool ABuiltinFiles::contains(const AString& file) {
    return inst().mBuffers.contains(file);
}
