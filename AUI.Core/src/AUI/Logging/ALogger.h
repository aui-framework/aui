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
		ERR
	};
private:
	ALogger();
	static ALogger& instance();
	AMutex mSync;
	_<FileOutputStream> mLogFile = _new<FileOutputStream>("latest.log");
	
	void log(Level level, const AString& str);

public:	
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
};
