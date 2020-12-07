#pragma once

#include "AUI/Core.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"

class AString;

class API_AUI_CORE BuiltinFiles
{
private:
	AMap<AString, _<AByteBuffer>> mBuffers;

	static BuiltinFiles& inst();
	BuiltinFiles() = default;

public:
	static void loadBuffer(AByteBuffer& data);
	static void load(const unsigned char* data, size_t size);
	static _<IInputStream> open(const AString& file);
};
