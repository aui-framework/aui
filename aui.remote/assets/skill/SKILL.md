---
name: aui-remote
description: Run, look at, click and type into a running AUI application (screenshot, view tree, input, logs) through its remote agent. Use to verify UI changes of an AUI app in a real window.
---

# Driving an AUI application

`{{EXE}} cli <command>` talks to the remote agent of an AUI application (module `aui::dev_extras`). Every command
prints one JSON object on the last line of stdout; on failure the exit code is non-zero and the object is
`{"error":{"code","message","data":{"hint"}}}`. Read `data.hint`.

The application must link `aui::dev_extras` (development builds only, never ship it) and call `ARemoteDebug::start()`
under `#if AUI_DEBUG` (see `examples/ui/remote_debug` in AUI); if it does not, add that first. The connection is `wss` to
127.0.0.1 with the certificate pinned from the app's discovery file; `list` shows `supported: false` for apps that
don't offer it.

## Start the application

```
{{EXE}} cli launch [--cwd dir] [--env K=V]... -- <exe> [args]
```

Prints `{pid, port, stdout, stderr}`. The app gets a trusted key in `AUI_REMOTE_KEY` (the app has to read it with
`ARemoteDebug::Options::fromEnvironment()`), no pairing needed. Stop it with `{{EXE}} cli stop <pid>`.

An application that was started by someone else (`./myapp`) must be paired once:

```
{{EXE}} cli list                      # running apps and whether they are paired
{{EXE}} cli pair <pid>                # the app shows a 6-digit code in its window
{{EXE}} cli pair <pid> --code 123456  # ask the user for the code
```

Never guess the code; after 3 wrong attempts it is void.

With several apps running pass `--pid N` to every command.

## The loop

1. On a screen you haven't seen yet, `screenshot --annotate`: the PNG has a numbered box over every clickable or
   typeable view, and `marks` maps each number to its ref. `interactive` lists the same without the picture;
   `find --type Button --text OK` searches.
2. Act: `click <ref>`, `click <x> <y>`, `type "text" [--ref R]`, `key Enter [--mod ctrl]`.
3. `wait-idle` after every input, then `screenshot` and look at the PNG whose path is in the result.
4. Fix the code, rebuild, `stop` and `launch` again.

## Commands

`{{EXE}} cli help` prints the same.

{{COMMANDS}}

Coordinates are logical pixels of the window (the screenshot may be scaled, see `scale` in its result).

## Rules

- Refs (`v<gen>:<id>`) go stale when the tree changes; on `STALE_REF` run `find` again.
- Always `wait-idle` after input before taking a screenshot.
- Check `logs` when something does not work.
- Look at the screenshot, do not assume that the UI looks as intended.

## Where to run the application

Prefer a headless environment if you can get one, so that (a) the application's windows don't pop up over the user's
work and (b) the user's mouse and keyboard don't interfere with you. It is a preference, not a requirement: not every
platform and desktop has one. Without one, or when the user asked to debug an application that is already open, work in
the user's environment and say so in your report.

Never inject OS-level input (XTest and alike) into the user's session without permission: it moves their cursor, types
into whatever window has focus and may make the desktop ask to allow remote control. The input of this tool is
injected into the application and does none of that.

Examples of headless environments are in `docs/agentic-development.md` of AUI.

## Notes

### Starting

- Start the application with `{{EXE}} cli launch`, not by setting `AUI_REMOTE_KEY` yourself: `launch` stores the key it passes,
  a key you invent is unknown to the client and every command fails with "no longer trusts this client".
- The user often has their own instance of the same app running. Run `cli list` first and pass `--pid` (taken from the
  output of `launch`) to every command. Write it out in each command instead of keeping `--pid N` in one shell variable:
  zsh does not split variables, so `$R cli windows $P` sends `--pid 123` as a single argument.
- `stop` what you launched when you are done; leftovers pile up in `list`.
- If the app exits right away, `launch` returns `stderrTail`; read it before rebuilding anything.

### Looking

- On a screen you haven't seen yet, take `screenshot --annotate`: every interactive view gets a numbered box on the
  picture, and `marks` maps the numbers to refs, so "click 7" is `click <ref of mark 7>` without matching coordinates.
- Start with `tree` (compact text) and `interactive`; use `tree --json` or `props <ref>` only when you need geometry or
  inspector fields. `hit <x> <y>` tells what is under a pixel you saw on a screenshot.
- A dialog is a window of its own: `type` and `key` go to the window of `--ref`, or `--window`, or the main one.
- Numbers don't prove that the UI is right; look at the picture. `resize` once reported the right size while the
  contents were still laid out for the old one, and only the screenshot showed it.
- `screenshot` returns `scale`: screenshot pixels divided by it are the window coordinates that `click` and `hit` take.
- Refs go stale after every `tree`/`find`/`interactive` call and after the UI changes. Re-run `find` on `STALE_REF`,
  don't reuse refs from an earlier step.
- An element of a long list may be outside of the visible part of its scroll area: `scroll-into-view <ref>` before
  clicking it or looking at it.

### Acting

- `wait-idle` after every input and every `resize`, then `screenshot`. Without it you see the frame before the change.
- `move` gives hover without a click (tooltips, hover styles); `press`, `move`, `release` give a drag with control over
  every step. Pointer commands use the pointer native to the app's platform (mouse on desktop, finger on Android and
  iOS); `--pointer finger` emulates touch on desktop. The GUI starts in the same mode and switches with M/T.
- Touch gestures (`long-press`, `fling`, `pinch`, `rotate`, `touch`) always use fingers and return when the last
  finger is lifted. After a `fling` the list keeps scrolling: `wait-idle` before the screenshot. Two-finger gestures
  reach views built on `A2FingerTransformArea`; elsewhere the fingers are just separate pointers.
- In T mode of the GUI, holding the finger still for half a second is a long press, as on a phone.
- To check responsive layouts, `resize` to a few sizes (a phone-like 360x740, a narrow desktop, a wide one) and take a
  screenshot of each. A size below the window's minimum or above 16384 is clamped; the result says to what.
- `scale 2` (or 0.75, 1.5, ...) shows the app as on a screen with another pixel density without changing the window
  size in pixels, unless the contents no longer fit: then the window grows to their minimum and keeps that size after
  `scale reset`, which returns to the system scale. The GUI has the same as the percentage next to the size.
- Read `logs --level warn` when something does not react: failed bindings, ASS errors and exceptions land there.

### Known gaps

- Native dialogs (file pickers, message boxes) are drawn by the OS, not by AUI: they are absent from `tree` and
  screenshots and don't take injected input. An agent that opens one is stuck until a human closes it. Avoid them in
  the flows an agent drives, or expose the action as a function (`ARemoteDebug::registerFunction`) that takes the path
  directly.
