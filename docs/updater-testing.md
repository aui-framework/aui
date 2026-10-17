# Testing and debugging AUpdater

## Unit-testing your UI

`checkForUpdatesImpl` and `downloadUpdateImpl` are pure virtual, so a GMock subclass is all that is needed to drive your
UI without network access. Assign `AUpdater::status` directly to simulate a state, and change `progress` of
`StatusDownloading` to simulate a download. The status view from [updater-ui] is tested this way in
`aui.updater/tests/UpdaterStatusTest.cpp`; combine it with [aui::uitests] (see [writing-tests]) to click on the
buttons.

```cpp
class MyUpdater : public AUpdater {
public:
    MOCK_METHOD(AFuture<void>, checkForUpdatesImpl, (), (override));
    MOCK_METHOD(AFuture<void>, downloadUpdateImpl, (const APath& unpackedUpdateDir), (override));
};
```

## Testing version logic

[aui::updater::Semver] and [aui::updater::AppropriatePortablePackagePredicate] are plain value types and are trivially
testable; feed them tag names and file names captured from your real releases.

## Testing the deployment

The deployment replaces your application's files, so it cannot be tested in-process. The framework's own test
(`UpdaterTest.ApplyUpdate`) launches a helper executable, `aui.updater.test`, with a `--test=<dir>` argument. A copy of
the executable plus a marker file is placed in a temporary directory in advance, which imitates a downloaded update. The
updater applies it, and the test checks the marker file appeared next to the original executable. To do the same in your
app, override `getUnpackedUpdateDir()` and `loadInstallCmdline()` (protected virtuals of [AUpdater]) so the updater picks
up a directory you prepared.

!!! note

    The deployment test is compiled only for static AUI builds.

## Debugging a failed update

Deployment is performed by a separate process after your application exits, so errors do not show up in your log. The
process id is logged by `applyUpdateAndRestart`, and the deployment log is the log file of that pid. See the
"Debugging update deployment" section of [AUpdater] for an annotated example. A typical failure is "Text file busy" or
"Access denied": the old process had not exited yet or an antivirus locks the file.

Common reasons why nothing happens:

- `AUpdater::isAvailable()` is false: the executable is not writable, or the platform is unsupported.
- The release has no archive matching the platform; `getQualifierDebug()` of the predicate prints what was expected.
- The archive is not a ZIP, or does not contain the executable under the name returned by `getModuleName()`.
