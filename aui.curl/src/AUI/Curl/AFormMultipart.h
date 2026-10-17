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
 * @ingroup curl
 * @details
 * A map from the field name to value (string, byte buffer or input stream, optionally with
 * `filename` and `mimeType`). Pass it to `ACurl::Builder::withMultipart` to upload form fields and files in a single
 * request; the `Content-Type` header with the boundary is set automatically.
 *
 * ```cpp
 * AFormMultipart form;
 * form["description"] = { .value = AString("My photo") };
 * form["file"] = { .value = _new<AFileInputStream>("photo.png"), .filename = AString("photo.png"), .mimeType = AString("image/png") };
 * ACurl::Builder("https://example.com/upload").withMethod(ACurl::Method::HTTP_POST).withMultipart(form).runBlocking();
 * ```
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
