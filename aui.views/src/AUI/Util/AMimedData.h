#pragma once

#include <AUI/Common/AMap.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Url/AUrl.h>

/**
 * @brief Mime-type data associated storage.
 * @ingroup views
 */
class API_AUI_VIEWS AMimedData {
public:
    AOptional<AString> text() const;

    /**
     * @brief Sets "text/plain".
     */
    void setText(const AString& text);


    void setData(const AString& mimeType, AByteBuffer data) {
        mStorage[mimeType] = std::move(data);
    }

    [[nodiscard]]
    const AMap<AString, AByteBuffer>& data() const noexcept {
        return mStorage;
    }

    AOptional<AVector<AUrl>> urls() const;
    void setUrls(const AVector<AUrl>& urls);

    void clear() noexcept {
        mStorage.clear();
    }

private:
    AMap<AString, AByteBuffer> mStorage;

    AOptional<AByteBufferView> findFirstOccurrence(const AStringVector& items) const {
        for (const auto& item : items) {
            if (auto c = mStorage.contains(item)) {
                return c->second;
            }
        }
        return std::nullopt;
    }
};


