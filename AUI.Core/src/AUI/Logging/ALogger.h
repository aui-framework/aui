#pragma once

#include <AUI/Core.h>
#include "AUI/Thread/AMutex.h"
#include "AUI/IO/FileOutputStream.h"

class AString;

class API_AUI_CORE ALogger
{
public:
	enum Level
	{
		INFO,
		WARN,
		ERR,
        DEBUG,
	};
private:
	ALogger();
	static ALogger& instance();
	AMutex mSync;
	_<FileOutputStream> mLogFile;
	
	void log(Level level, const AString& str);

public:
    static void setLogFile(const AString& str);

	static void info(const AString& str)
	{
		instance().log(INFO, str);
	}	
	static void warn(const AString& str)
	{
		instance().log(WARN, str);
	}	
	static void err(const AString& str)
	{
		instance().log(ERR, str);
	}
	static void debug(const AString& str)
	{
		instance().log(DEBUG, str);
	}
};
