#pragma once

#include <AUI/Common/AMap.h>
#include "AUI/Common/AByteBuffer.h"

/**
 * @brief Mime-type data associated storage.
 * @ingroup views
 */
class API_AUI_VIEWS AMimedData {
public:
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


private:
    AMap<AString, AByteBuffer> mStorage;
};


