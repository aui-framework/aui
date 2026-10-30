---
name: aui-remote
description: Run, look at, click and type into a running AUI application through its remote agent (aui::dev_extras). Use to verify UI changes in a real window.
---

# Driving an AUI application

1. The application must link `aui::dev_extras` (development builds only) and call `ARemoteDebug::start()` under
   `#if AUI_DEBUG`; see `examples/ui/remote_debug`. If it does not, add it.
2. Build the client: `cmake --build build -t aui.remote`. It is `build/bin/aui-remote`, on macOS
   `build/bin/aui-remote.app/Contents/MacOS/aui-remote`. Below it is `aui-remote`.
3. Start the application with `aui-remote cli launch -- <exe> [args]`, which needs no pairing, and stop it with
   `aui-remote cli stop <pid>`. One started by someone else has to be paired once: `aui-remote cli pair <pid>` makes it
   show a 6-digit code; ask the user for it and run `aui-remote cli pair <pid> --code <code>`. Never guess the code.
4. Loop: `screenshot --annotate` on a screen you haven't seen (numbered boxes, `marks` maps them to refs), `click <ref>`,
   `type "text" [--ref R]`, `key Enter`, `wait-idle`, `screenshot` (look at the PNG), fix the code, rebuild, repeat.

Every command prints one JSON object on the last line of stdout; errors are `{"error":{"code","message","data":{"hint"}}}`.
With several applications running pass `--pid N`. All the commands and options are in `aui.remote/assets/skill/SKILL.md`,
the protocol is in `docs/agentic-development.md`.

Rules:
- Refs (`v<gen>:<id>`) go stale after the tree changes; on `STALE_REF` call `find` again.
- Always `wait-idle` after input before taking a screenshot.
- A dialog is a window of its own: `type` and `key` go to the window of `--ref`, or `--window`, or the main one.
- Errors contain `data.hint`; read it.
- Never ship `aui::dev_extras` in release builds.
