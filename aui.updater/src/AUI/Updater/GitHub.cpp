//
// Created by alex2772 on 1/25/25.
//

#include "GitHub.h"
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/Conversion.h>

// clang-format off
AJSON_FIELDS(aui::updater::github::LatestReleaseResponse::Asset,
     AJSON_FIELDS_ENTRY(browser_download_url)
     AJSON_FIELDS_ENTRY(name)
)

AJSON_FIELDS(aui::updater::github::LatestReleaseResponse,
    AJSON_FIELDS_ENTRY(id)
    AJSON_FIELDS_ENTRY(tag_name)
    AJSON_FIELDS_ENTRY(body)
    AJSON_FIELDS_ENTRY(prerelease)
    AJSON_FIELDS_ENTRY(assets)
)
// clang-format on

aui::updater::github::LatestReleaseResponse
aui::updater::github::latestRelease(const AString& repoOwner, const AString& repoName) {
    // example response:
    //{
    //  "url": "https://api.github.com/repos/octocat/Hello-World/releases/1",
    //  "html_url": "https://github.com/octocat/Hello-World/releases/v1.0.0",
    //  "assets_url": "https://api.github.com/repos/octocat/Hello-World/releases/1/assets",
    //  "upload_url": "https://uploads.github.com/repos/octocat/Hello-World/releases/1/assets{?name,label}",
    //  "tarball_url": "https://api.github.com/repos/octocat/Hello-World/tarball/v1.0.0",
    //  "zipball_url": "https://api.github.com/repos/octocat/Hello-World/zipball/v1.0.0",
    //  "discussion_url": "https://github.com/octocat/Hello-World/discussions/90",
    //  "id": 1,
    //  "node_id": "MDc6UmVsZWFzZTE=",
    //  "tag_name": "v1.0.0",
    //  "target_commitish": "master",
    //  "name": "v1.0.0",
    //  "body": "Description of the release",
    //  "draft": false,
    //  "prerelease": false,
    //  "created_at": "2013-02-27T19:35:32Z",
    //  "published_at": "2013-02-27T19:35:32Z",
    //  "author": {
    //    "login": "octocat",
    //    "id": 1,
    //    "node_id": "MDQ6VXNlcjE=",
    //    "avatar_url": "https://github.com/images/error/octocat_happy.gif",
    //    "gravatar_id": "",
    //    "url": "https://api.github.com/users/octocat",
    //    "html_url": "https://github.com/octocat",
    //    "followers_url": "https://api.github.com/users/octocat/followers",
    //    "following_url": "https://api.github.com/users/octocat/following{/other_user}",
    //    "gists_url": "https://api.github.com/users/octocat/gists{/gist_id}",
    //    "starred_url": "https://api.github.com/users/octocat/starred{/owner}{/repo}",
    //    "subscriptions_url": "https://api.github.com/users/octocat/subscriptions",
    //    "organizations_url": "https://api.github.com/users/octocat/orgs",
    //    "repos_url": "https://api.github.com/users/octocat/repos",
    //    "events_url": "https://api.github.com/users/octocat/events{/privacy}",
    //    "received_events_url": "https://api.github.com/users/octocat/received_events",
    //    "type": "User",
    //    "site_admin": false
    //  },
    //  "assets": [
    //    {
    //      "url": "https://api.github.com/repos/octocat/Hello-World/releases/assets/1",
    //      "browser_download_url": "https://github.com/octocat/Hello-World/releases/download/v1.0.0/example.zip",
    //      "id": 1,
    //      "node_id": "MDEyOlJlbGVhc2VBc3NldDE=",
    //      "name": "example.zip",
    //      "label": "short description",
    //      "state": "uploaded",
    //      "content_type": "application/zip",
    //      "size": 1024,
    //      "download_count": 42,
    //      "created_at": "2013-02-27T19:35:32Z",
    //      "updated_at": "2013-02-27T19:35:32Z",
    //      "uploader": {
    //        "login": "octocat",
    //        "id": 1,
    //        "node_id": "MDQ6VXNlcjE=",
    //        "avatar_url": "https://github.com/images/error/octocat_happy.gif",
    //        "gravatar_id": "",
    //        "url": "https://api.github.com/users/octocat",
    //        "html_url": "https://github.com/octocat",
    //        "followers_url": "https://api.github.com/users/octocat/followers",
    //        "following_url": "https://api.github.com/users/octocat/following{/other_user}",
    //        "gists_url": "https://api.github.com/users/octocat/gists{/gist_id}",
    //        "starred_url": "https://api.github.com/users/octocat/starred{/owner}{/repo}",
    //        "subscriptions_url": "https://api.github.com/users/octocat/subscriptions",
    //        "organizations_url": "https://api.github.com/users/octocat/orgs",
    //        "repos_url": "https://api.github.com/users/octocat/repos",
    //        "events_url": "https://api.github.com/users/octocat/events{/privacy}",
    //        "received_events_url": "https://api.github.com/users/octocat/received_events",
    //        "type": "User",
    //        "site_admin": false
    //      }
    //    }
    //  ]
    //}

    return aui::from_json<aui::updater::github::LatestReleaseResponse>(AJson::fromBuffer(
        ACurl::Builder("https://api.github.com/repos/{}/{}/releases/latest"_format(repoOwner, repoName))
            .withHeaders({
              "Accept: application/json",
              "X-GitHub-Api-Version: 2022-11-28",
              "User-Agent: {}"_format(repoOwner),
            })
            .runAsync()->body));
}
