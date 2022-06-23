#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>

static constexpr auto LOG_TAG = "MyApp";

AUI_ENTRY {
    ALogger::info(LOG_TAG) << "Hello world!";
    return 0;
}