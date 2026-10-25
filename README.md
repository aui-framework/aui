# aui-remote

Client of the remote agent of [AUI](https://github.com/aui-framework/aui) applications (`aui::dev_extras`). It lets a
human or an AI coding agent look at a running AUI app, click and type into it, read its log and compare screenshots.

- GUI (`aui-remote`): running applications, pairing, view tree and screenshot of the selected one, and the checkbox
  "Let Agents drive AUI remote" that installs the Claude Code skill.
- CLI (`aui-remote cli <command>`): the same through the command line. Every command prints one JSON object on the
  last line of stdout, logs go to stderr, the exit code is non-zero on errors (the object is then `{"error": {...}}`).

The application under test links `aui::dev_extras` (development builds only) and runs with `AUI_REMOTE=1`; see
`docs/agentic-development.md` of AUI.

## CLI

```
aui-remote cli list
aui-remote cli launch [--cwd dir] [--env K=V]... [--timeout s] -- <exe> [args]
aui-remote cli stop <pid>
aui-remote cli pair <pid>                 # the application shows a 6-digit code
aui-remote cli pair <pid> --code 123456
aui-remote cli windows | tree | find | props | hit | interactive | shot
aui-remote cli click | dblclick | press | release | move | drag | scroll
aui-remote cli type | key | wait-idle | wait-view | logs | functions | call
aui-remote cli compare a.png b.png [--threshold 0..255]
```

The target application is `--pid N`, or the only one running. Each command opens a connection, does its job and closes
it. The full list of options is in [SKILL.md](assets/skill/SKILL.md), which is also what the agent reads.

### Pairing and keys

Applications started with `launch` get a key through `AUI_REMOTE_KEY` and need no pairing. For any other application,
`pair` asks it to show a 6-digit code (in its window and log); `pair --code` confirms it and stores a random key in
`~/.aui/remote/clients/<pid>-<startedAt>.key` (mode 0600). The key is valid until the application exits.

Applications are discovered through `~/.aui/remote/<pid>.json`; files of dead processes are deleted by `list`.

### Transport

The agent speaks JSON-RPC 2.0 over `wss://127.0.0.1:<port>/?key=<key>` with a self-signed certificate. The discovery
file carries `scheme` (`wss`) and `certSha256`; the client connects only if the SHA-256 of the presented certificate
equals `certSha256` (no CA chain is verified). Agents whose discovery file lacks either are refused. The client is
implemented on `ATlsSession` (aui::crypt) over `ATcpSocket` with a hand-written WebSocket handshake and the frame codec of
aui::curl, because `AWebsocket` can't pin certificates.

## Building

```bash
cmake -B build -GNinja
cmake --build build -j8
cmake --build build -j8 --target Tests && ./build/bin/Tests
```

By default AUI is downloaded by AUI.Boot (`AUI_VERSION` in `CMakeLists.txt`). To build against a local AUI checkout:

```bash
cmake -B build -GNinja -DAUI_LOCAL_PATH=$HOME/CLionProjects/aui
```

In this mode AUI is imported with `add_subdirectory`, so the AUI tests are built into `Tests` as well. Drop the flag (and
bump `AUI_VERSION`) to return to a released AUI.

Needs an AUI with `ATlsSession` (aui::crypt) and the WebSocket frame codec of aui::curl. The tests use `AWebsocketServer` with a
self-signed `ATlsIdentity` as a stand-in for the agent.

## Claude Code skill

Tick "Let Agents drive AUI remote" to write `($CLAUDE_CONFIG_DIR or ~/.claude)/skills/aui-remote/SKILL.md`; untick to
remove it. The state of the checkbox is the existence of that file. The skill is built from
`assets/skill/SKILL.md`, with `{{EXE}}` replaced by the path to this executable.
