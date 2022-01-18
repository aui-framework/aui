#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include <zlib.h>

AUI_ENTRY {
    ALogger::info("Hello world!");
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    inflateInit(&strm);
    return 0;
}