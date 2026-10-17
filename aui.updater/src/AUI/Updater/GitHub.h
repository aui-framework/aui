#pragma once

#include <AUI/Common/AObject.h>

namespace aui::updater::github {
/**
 * @brief Subset of the GitHub "latest release" REST API response.
 * @ingroup updater
 * @details
 * `tag_name` is the tag of the release (i.e., `v1.2.3`), `assets` are the files attached to the release.
 */
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

/**
 * @brief Queries the latest published release of a GitHub repository.
 * @param repoOwner user or organization name.
 * @param repoName repository name.
 * @return the latest release.
 * @details
 * Performs a blocking HTTPS request to `api.github.com`, so call it from a background thread (i.e., from
 * `AUpdater::checkForUpdatesImpl`). Throws an exception on network or parse errors. Drafts and prereleases are not
 * returned by GitHub for this endpoint.
 */
API_AUI_UPDATER LatestReleaseResponse latestRelease(const AString& repoOwner, const AString& repoName);
}
