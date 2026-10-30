# aui-remote

Client of the remote agent of AUI applications (`aui::dev_extras`), module `aui.remote` of AUI. It lets a human or an AI
coding agent look at a running AUI app, click and type into it, read its log and compare screenshots.

- GUI (`aui-remote`): running applications, pairing, the live frame of the selected one (scaled down to fit, the rulers
  show the pixels of the viewport then) with the input forwarded to it,
  and the checkbox "Allow Claude to use AUI Remote" that installs the Claude Code skill. The view tree works like the
  in-process devtools: hovering a view draws its content, padding and margin over the frame, clicking it shows its
  properties, and moving the pointer over the frame with CTRL held picks the view under it.
- CLI (`aui-remote cli <command>`): the same through the command line. Every command prints one JSON object on the
  last line of stdout, logs go to stderr, the exit code is non-zero on errors (the object is then `{"error": {...}}`).

The application under test links `aui::dev_extras` (development builds only) and starts `ARemoteDebug` from its code,
typically `ARemoteDebug::start(ARemoteDebug::Options::fromEnvironment())` under `#if AUI_DEBUG`. Nothing is listening
otherwise. See `docs/agentic-development.md` and `examples/ui/remote_debug` of AUI.

## CLI

`aui-remote cli help` lists the commands with their options. Every command opens a connection, does its job and closes
it; the target application is `--pid N`, or the only one running.

How it works (pairing, keys, encryption, protocol, headless environments) is in
[Agentic development](../docs/agentic-development.md). What an AI agent reads is
[assets/skill/SKILL.md](assets/skill/SKILL.md).

## Building

`aui.remote` is built with AUI when the modules it needs are (views, curl, json, image, crypt, network) and AUI is not
cross-compiled:

```bash
cmake -B build -GNinja
cmake --build build -j8 -t aui.remote          # build/bin/aui-remote, on macOS build/bin/aui-remote.app/Contents/MacOS/aui-remote
cmake --build build -j8 -t Tests && ./build/bin/Tests --gtest_filter='EndToEndTest.*:CliTest.*'
```

Its tests run in the `Tests` of AUI. `EndToEndTest` drives the real agent of `aui.dev_extras` with the CLI, so a change
of the protocol has to keep both sides in step; `FakeAgent` stands in for the agent where a test needs to control it.

In a consumer project, `cmake -P aui.boot.cmake remote build` prints the path to the `aui-remote` of the AUI the project
uses. With `-DAUIB_AUI_AS=ON` AUI is a part of the consumer's build: `aui.remote` is then excluded from all and built
with `cmake --build build -t aui.remote`.

## Claude Code skill

Tick "Allow Claude to use AUI Remote" to write `($CLAUDE_CONFIG_DIR or ~/.claude)/skills/aui-remote/SKILL.md`; untick to
remove it. The state of the checkbox is the existence of that file. The skill is built from
`assets/skill/SKILL.md`, with `{{EXE}}` replaced by the path to this executable and `{{COMMANDS}}` by the table of
commands from `src/cli/Args.cpp`.
