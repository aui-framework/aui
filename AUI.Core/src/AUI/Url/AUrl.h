#pragma once
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include <AUI/Common/AMap.h>

class API_AUI_CORE AUrl
{
private:
	AString mProtocol;
	AString mHost;
	AString mPath;


    static AMap<AString, std::function<_<IInputStream>(const AUrl&)>> ourResolvers;

public:
	AUrl(const AString& full);
	inline AUrl(const char* full): AUrl(AString(full)) {}

	_<IInputStream> open() const;
	
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

	AString getFull() const {
	    return mProtocol + "://" + mHost + "/" + mPath;
	}

	static void registerResolver(const AString& protocol, const std::function<_<IInputStream>(const AUrl&)>& factory);
};
