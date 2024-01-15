#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/IO/IInputStream.h>

struct AFormMultipartEntry {
    std::variant<_<IInputStream>, AString, AByteBuffer> value;
    AOptional<AString> filename;
    AOptional<AString> mimeType;
};

/**
 * @brief Web multipart/form-data representation.
 */
class AFormMultipart: public AMap<AString, AFormMultipartEntry> {
public:
    using AMap<AString, AFormMultipartEntry>::AMap;

    _<IInputStream> makeInputStream() const;

    [[nodiscard]]
    const AString& boundary() const noexcept {
        return mBoundary;
    }


private:
    mutable AString mBoundary;

};
