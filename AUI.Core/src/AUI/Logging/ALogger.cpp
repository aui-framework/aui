#include "ALogger.h"
#ifdef __ANDROID__
#include <android/log.h>
#else
#include <ctime>
#endif

ALogger::ALogger()
{

#ifndef __ANDROID__
    try {
        mLogFile = _new<FileOutputStream>("latest.log");
    } catch (const AException& e) {
        log(WARN, e.getMessage());
    }
#endif
}

ALogger& ALogger::instance()
{
	static ALogger l;
	return l;
}

void ALogger::log(Level level, const AString& str)
{
#ifdef __ANDROID__

    int prio;
    switch (level) {
        case INFO:
            prio = ANDROID_LOG_INFO;
            break;
        case WARN:
            prio = ANDROID_LOG_WARN;
            break;
        case ERR:
            prio = ANDROID_LOG_ERROR;
            break;
        case DEBUG:
            prio = ANDROID_LOG_DEBUG;
            break;
        default:
            assert(0);
    }
    __android_log_print(prio, "AUI", "%ls", str.c_str());
#else
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
    case DEBUG:
        levelName = "DEBUG";
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
#endif
}