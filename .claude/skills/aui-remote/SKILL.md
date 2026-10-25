---
name: aui-remote
description: Run, look at, click and type into a running AUI application through its remote agent (aui::dev_extras). Use to verify UI changes in a real window.
---

# Driving an AUI application

1. The application must link `aui::dev_extras` (development builds only). Start it with `AUI_REMOTE=1`.
2. Find it in `~/.aui/remote/<pid>.json` (`port`, `token`). Connect to `ws://127.0.0.1:<port>/?token=<token>`
   and speak JSON-RPC 2.0, or use `aui-remote cli` if installed.
3. Loop: `view.interactive` (what can be clicked) -> `input.pointer` / `input.type` / `input.key` -> `wait.idle`
   -> `screenshot` (returns a PNG path; look at the file) -> fix the code, rebuild, repeat.

Methods: `session.hello`, `window.list`, `view.tree`, `view.find`, `view.properties`, `view.hitTest`,
`view.interactive`, `screenshot`, `input.pointer`, `input.type`, `input.key`, `wait.idle`, `wait.view`,
`diag.log`, `fn.list`, `fn.call`. Full description: `docs/agentic-development.md`.

Rules:
- Refs (`v<gen>:<id>`) go stale after the tree changes; on `STALE_REF` call `view.find` again.
- Always `wait.idle` after input before taking a screenshot.
- Errors contain `data.hint`; read it.
- Never ship `aui::dev_extras` in release builds.
