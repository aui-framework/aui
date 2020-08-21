#pragma once

#include "Singleton.h"
#include "AUI/Core.h"
#include "AUI/Common/ByteBuffer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"

class AString;

class API_AUI_CORE BuiltinFiles: Singleton<BuiltinFiles>
{
private:
	AMap<AString, _<ByteBuffer>> mBuffers;
	
public:
	static void loadBuffer(ByteBuffer& data);
	static void load(const unsigned char* data, size_t size);
	static _<IInputStream> open(const AString& file);
};
