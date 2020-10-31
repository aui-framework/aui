#include "AUrl.h"

#include <functional>

#include "AUI/Common/AMap.h"
#include "AUI/Util/BuiltinFiles.h"

AUrl::AUrl(const AString& full)
{
	auto posColon = full.find(':');
	if (posColon == AString::NPOS)
	{
		mPath = full;
		mProtocol = "file";
	} else
	{
		mProtocol = full.mid(0, posColon);
		if (mProtocol.empty())
		{
			mProtocol = "builtin";
			mPath = full.mid(posColon + 1);
		} else
		{
			auto posHost = posColon + 3;
			auto posSlash = full.find('/', posHost);
			if (posSlash == AString::NPOS)
				posSlash = 0;
			mHost = full.mid(posHost, posSlash - posHost);
			if (posSlash)
			{
				mPath = full.mid(posSlash + 1);
			}
		}
	}
}

AMap<AString, std::function<_<IInputStream>(const AUrl&)>> AUrl::ourResolvers = {
        {"builtin", [](const AUrl& u) {
            return BuiltinFiles::open(u.getPath());
        }}
};
_<IInputStream> AUrl::open() const {
	if (auto is = ourResolvers[mProtocol](*this))
	    return is;
	throw IOException("could not open url: " + getFull());
}

void AUrl::registerResolver(const AString& protocol, const std::function<_<IInputStream>(const AUrl&)>& factory) {
    ourResolvers[protocol] = factory;
}
