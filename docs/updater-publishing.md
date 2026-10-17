# Publishing releases for AUpdater

[AUpdater] does not impose where the update comes from. What it needs is a ZIP archive with a complete copy of your
application, found by `checkForUpdatesImpl` and downloaded by `downloadUpdateImpl`. This page shows the setup that
[app-template] uses: GitHub releases.

## Produce the archive

The update is always a ZIP portable. Build it with `AUI_PORTABLE_ZIP` packaging, see [packaging]:

```sh
cmake .. -DCMAKE_BUILD_TYPE=Release -DAUI_APP_PACKAGING=AUI_PORTABLE_ZIP
cmake --build . --config Release
cpack . -c Release -B artifacts
gh release upload ${{ github.ref }} artifacts/*.*
```

The archive name contains the platform and the architecture, followed by `-portable`, i.e.
`myapp-1.2.3-linux-x86_64-portable.zip`. Upload the archive for every platform and architecture you support to the same
release. `tar.gz` archives are not supported by the updater.

If you distribute a Windows installer as well, it has to install into the user's directory so the updater can replace
files without elevation; see [INNOSETUP].

## Pick the right asset

[aui::updater::AppropriatePortablePackagePredicate] tells whether a file name is a portable for the current platform
and architecture. It is meant to be used with `ranges::find_if`. If you name archives differently, pass your own
qualifier:

<!-- aui:snippet aui.updater/tests/AppropriatePredicateTest.cpp predicate_qualifier -->

## Check GitHub and download

`aui::updater::github::latestRelease` (see [aui::updater::github::LatestReleaseResponse]) returns the latest non-draft, non-prerelease release of a repository together
with its assets. The example below compares the tag with the current version, finds the matching asset, asks the user
and starts the download.

<!-- aui:snippet aui.updater/tests/UpdaterTest.cpp updater_github -->

`downloadAndUnpack` downloads the archive, reports progress to `AUpdater::status` and unpacks into
`unpackedUpdateDir`. `reportReadyToApplyAndRestart` then switches the status to
`AUpdater::StatusWaitingForApplyAndRestart` and remembers the command line to launch, so even if the user quits the app
now, the update is applied on the next start.

!!! warning

    Anonymous GitHub API requests are rate-limited per IP address. Do not check for updates more often than once per
    several hours, and never in a loop.

## Other sources

Nothing in `AUpdater` is tied to GitHub. Fetch a JSON manifest from your server in `checkForUpdatesImpl`, remember the
download URL in a member, and pass it to `downloadAndUnpack` in `downloadUpdateImpl`. Only a ZIP archive is understood
by `downloadAndUnpack`; for other formats, unpack into `unpackedUpdateDir` yourself.
