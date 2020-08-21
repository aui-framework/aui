#include "ALogger.h"
#include <ctime>

ALogger::ALogger()
{
}

ALogger& ALogger::instance()
{
	static ALogger l;
	return l;
}

void ALogger::log(Level level, const AString& str)
{
    const char* levelName = "UNKNOWN";

    switch (level)
    {
    case INFO:
        levelName = "INFO";
        break;
    case WARN:
        levelName = "WARN";
        break;
    case ERR:
        levelName = "ERR";
        break;
    }

    std::time_t t = std::time(nullptr);
    std::tm* tm;
    tm = localtime(&t);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm);

    std::unique_lock lock(mSync);

    std::cout << '[' << timebuf << "] " << levelName << ": " << str << std::endl;
    *mLogFile << '[';
    mLogFile->write(timebuf, strlen(timebuf));
	*mLogFile << "] ";
    mLogFile->write(levelName, strlen(levelName));
    *mLogFile << ": " << str << "\n";
}