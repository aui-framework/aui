#pragma once
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"

class API_AUI_CORE AUrl
{
private:
	AString mProtocol;
	AString mHost;
	AString mPath;
	
public:
	AUrl(const AString& full);

	_<IInputStream> open();
	
	const AString& getHost() const
	{
		return mHost;
	}
	const AString& getPath() const
	{
		return mPath;
	}
	const AString& getProtocol() const
	{
		return mProtocol;
	}
	
};
