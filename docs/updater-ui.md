# Update UI: progress, apply and restart

[AUpdater] exposes everything the UI needs as one property, `AUpdater::status`. It holds one of the statuses
`AUpdater::StatusIdle`, `AUpdater::StatusCheckingForUpdates`, `AUpdater::StatusDownloading`,
`AUpdater::StatusWaitingForApplyAndRestart` or `AUpdater::StatusNotAvailable` (or your own type) in a `std::any`.
It is updated in the UI thread only.

## Status-driven view

Project the status into views with `readProjected`: each time the status changes, the view is rebuilt. The downloading
status carries a `progress` property that can be bound directly to [AProgressBar]; the bar then updates without
rebuilding.

<!-- aui:snippet aui.updater/tests/UpdaterStatusTest.cpp updater_status_ui -->

The buttons call `checkForUpdates` and `applyUpdateAndRestart`. `applyUpdateAndRestart` terminates the process, so
save the user's data before calling it.

## Custom statuses

`status` is `AProperty<std::any>`, so you may assign your own struct, for example to display an error after a failed
download. The updater does not interpret unknown types.

```cpp
struct StatusFailed {
    AString reason;
};
updater->status = StatusFailed { "No internet connection" };
```

Reset the status to `AUpdater::StatusIdle` when the user dismisses the error, otherwise `checkForUpdates` stays
unavailable (it requires `StatusIdle`).

## Choosing the workflow

Whether to ask the user at every step or to download silently is up to you. See "Updater workflows" in [updater].
When downloading silently, call `downloadUpdate()` at the end of `checkForUpdatesImpl`, and show only a "Restart to
update" button for `StatusWaitingForApplyAndRestart`. Avoid modal message boxes unless the user explicitly asked for the
check.

## After the update

On the first start of the updated application `handleStartup` removes the downloaded files. If the deployment failed,
the old version is launched with `--aui-updater-failed` and the reason is available from
`AUpdater::getLastDeploymentError()`, which you can show in the UI. See [updater-testing] for debugging.
