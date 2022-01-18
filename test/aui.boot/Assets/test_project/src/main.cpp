#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Url/AUrl.h>

AUI_ENTRY {
    auto buf = AByteBuffer::fromStream(AUrl(":test.txt").open());
    return std::memcmp(buf.data(), "azaza", 5);
}