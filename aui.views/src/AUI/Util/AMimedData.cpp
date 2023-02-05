// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

//
// Created by Alex2772 on 9/15/2022.
//

#include <numeric>
#include "AMimedData.h"

void AMimedData::setText(const AString& text) {
    setData("text/plain", AByteBuffer::fromString(text));
}

AOptional<AString> AMimedData::text() const {
    if (auto v = findFirstOccurrence({"text/plain","text/utf8",})) {
        return AString::fromUtf8(*v);
    }
    return std::nullopt;
}

AOptional<AVector<AUrl>> AMimedData::urls() const {
    if (auto v = findFirstOccurrence({"text/uri-list",})) {
        return AString::fromUtf8(*v).split('\n').filter([](const AString& url) { return !url.empty(); }).map([](const AString& entry) {
            return AUrl(entry);
        });
    }
    return std::nullopt;
}

void AMimedData::setUrls(const AVector<AUrl>& urls) {
    AByteBuffer buffer(std::accumulate(urls.begin(), urls.end(), 0llu, [](auto lhs, const AUrl& url) {
        return lhs + url.path().length() + url.schema().length() + 6; // 6 for \n and ://
    }));

    for (const auto& url : urls) {
        auto full = url.full().toStdString();
        buffer.write(full.c_str(), full.length());
        buffer.write("\n", 1);
    }

    setData("text/uri-list", std::move(buffer));
}

