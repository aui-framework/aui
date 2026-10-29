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

1. `interactive` lists what can be clicked or typed into (with refs); `find --type Button --text OK` searches.
2. Act: `click <ref>`, `click <x> <y>`, `type "text" [--ref R]`, `key Enter [--mod ctrl]`.
3. `wait-idle` after every input, then `screenshot` and look at the PNG whose path is in the result.
4. Fix the code, rebuild, `stop` and `launch` again.

## Commands

| Command                                                         | Purpose                                               |
|-----------------------------------------------------------------|-------------------------------------------------------|
| `windows`                                                       | windows and their ids                                 |
| `resize <width> <height> [--window W]`                          | resizes the window, prints the size it got            |
| `scale <factor\|reset> [--window W]`                            | DPI scale on top of the system one (0.25..4; `reset` = 1) |
| `tree [--ref R] [--depth N] [--json] [--inspector] [--window W]` | view hierarchy, compact text (`--json` or `--inspector`: JSON) |
| `find [--type T] [--text S] [--ass NAME] [--ref R] [--interactive] [--include-invisible] [--window W]` | search views |
| `props <ref>`                                                   | properties of a view                                  |
| `hit <x> <y> [--window W]`                                      | views under a point                                   |
| `scroll-into-view <ref>`                                        | scrolls the scroll areas so the view is visible; prints `visible` |
| `interactive [--window W]`                                      | clickable and typeable views                          |
| `screenshot [--window W] [--ref R] [--padding N]`               | saves a PNG, prints `path`; `--ref` crops to a view   |
| `click\|dblclick\|press\|release\|move <ref \| x y> [--pointer mouse\|finger] [--button b] [--mod m]...` | pointer; without `--pointer` the app uses its native one (mouse on desktop, finger on Android/iOS) |
| `drag <ref \| x y> <ref \| x y>`                                | drag from one point to another                        |
| `scroll <ref \| x y> --dx N --dy N`                             | wheel                                                 |
| `long-press <ref \| x y> [--duration ms]`                       | touch and hold; prints `handled`                      |
| `fling <ref \| x y> [--dx N] [--dy N] [--velocity px/s]`        | swipe and lift, the list keeps scrolling; `wait-idle` waits for it |
| `pinch <ref \| x y> --from D --to D [--angle deg]`              | two fingers, distance from D to D (zoom)              |
| `rotate <ref \| x y> --degrees N [--distance D]`                | two fingers turning around the point                  |
| `touch --fingers '[{"finger":0,"points":[{"x":1,"y":2,"t":0},...]}]'` | any multi-touch: finger tracks, `t` in ms        |
| `type <text> [--ref R]`, `key <key> [--mod m]... [--ref R]`     | keyboard                                              |
| `wait-idle [--timeout ms] [--quiet ms]`                         | until nothing is happening                            |
| `wait-view <find filters> [--state visible\|gone] [--timeout ms]` | until a view appears or disappears                  |
| `logs [--since N] [--level L] [--limit N]`                      | log of the app; `next` of the result is the next `since` |
| `functions`, `call <name> [--args '{"k":1}']`                   | application specific functions                        |
| `compare <a.png> <b.png> [--threshold 0..255]`                  | percentage of different pixels                        |

Coordinates are logical pixels of the window (the screenshot may be scaled, see `scale` in its result).

## Rules

- Refs (`v<gen>:<id>`) go stale when the tree changes; on `STALE_REF` run `find` again.
- Always `wait-idle` after input before taking a screenshot.
- Check `logs` when something does not work.
- Look at the screenshot, do not assume that the UI looks as intended.
