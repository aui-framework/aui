// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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


