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

