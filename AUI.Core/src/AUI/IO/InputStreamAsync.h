#pragma once
#include "IInputStream.h"
#include "AUI/Common/ASignal.h"

class API_AUI_CORE InputStreamAsync: public AObject
{
private:
	_<AThread> mReadThread;
	
public:
	InputStreamAsync(_<IInputStream> inputStream);
	
signals:
	emits<_<ByteBuffer>> read;
	emits<> finished;
};
