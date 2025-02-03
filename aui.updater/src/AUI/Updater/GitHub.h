#pragma once

#include <AUI/Common/AObject.h>

namespace aui::updater::github {
struct LatestReleaseResponse {
    int32_t id;
    AString tag_name;
    AString body;
    bool prerelease;

    struct Asset {
        AString browser_download_url;
        AString name;
    };
    AVector<Asset> assets;
};

API_AUI_UPDATER LatestReleaseResponse latestRelease(const AString& repoOwner, const AString& repoName);
}
