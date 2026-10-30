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

```
aui-remote cli list
aui-remote cli launch [--cwd dir] [--env K=V]... [--timeout s] -- <exe> [args]
aui-remote cli stop <pid>
aui-remote cli pair <pid>                 # the application shows a 6-digit code
aui-remote cli pair <pid> --code 123456
aui-remote cli windows | resize | scale | scroll-into-view | tree | find | props | hit | interactive | screenshot
aui-remote cli click | dblclick | press | release | move | drag | scroll
aui-remote cli long-press | fling | pinch | rotate | touch
aui-remote cli type | key | wait-idle | wait-view | logs | functions | call
aui-remote cli compare a.png b.png [--threshold 0..255]
```

The target application is `--pid N`, or the only one running. Each command opens a connection, does its job and closes
it. The full list of options is in [SKILL.md](assets/skill/SKILL.md), which is also what the agent reads.

### Pairing and keys

Keys live in `~/.aui/remote/clients` (mode 0600). There are two kinds:

- the session key, `<pid>-<startedAt>.session`: valid until the application process exits. `launch` generates one and
  passes it in `AUI_REMOTE_KEY` (picked up by `Options::fromEnvironment()`), so launched applications need no pairing;
- the paired key, `<exe name>-<hash of exe path>.key`: valid across restarts of the same executable.

For any other application, `pair <pid>` asks it to show a 6-digit code (in its window and log) and `pair <pid> --code`
confirms it with a new random key. What is kept depends on the pairing policy of the application
(`ARemoteDebug::Options::pairing`, or `AUI_REMOTE_PAIRING`):

| Policy               | The application                       | The client keeps |
|----------------------|---------------------------------------|------------------|
| `remember` (default) | trusts the client after restarts      | the paired key   |
| `session`            | trusts the client until it exits      | a session key    |
| `always`             | asks for the code on every connection | nothing          |

Every CLI command is a connection of its own, so `always` is usable from the GUI only. The paired key survives a run of
the application with `session` or `always`: it is deleted only when an application with `remember` rejects it.

`pair` on an application that already accepts the stored key answers `alreadyPaired` without asking for a code. When an
application rejects a stored key (it forgot the client), the key is deleted and the error says to pair again.

Applications are discovered through `~/.aui/remote/<pid>.json`. `list` deletes the discovery files and the session keys
of dead processes; paired keys are kept.

### Transport

The agent speaks JSON-RPC 2.0 over `wss://127.0.0.1:<port>/?key=<key>` with a self-signed certificate. The discovery
file carries `scheme` (`wss`) and `certSha256`; the client connects only if the SHA-256 of the presented certificate
equals `certSha256` (no CA chain is verified). Agents whose discovery file lacks either are refused. The client is
implemented on `ATlsSession` (aui::crypt) over `ATcpSocket` with a hand-written WebSocket handshake and the frame codec of
aui::curl, because `AWebsocket` can't pin certificates.

## Working with aui-remote as an agent

Notes from agents that used it to develop AUI applications, `aui-remote` itself included.

**Where to run the application**

Run the application in a headless environment by default, so that (a) its windows don't pop up over the user's work
and (b) the user's mouse and keyboard don't interfere with the agent. Work in the user's environment only:

- when the user asked to debug an application that is already open, or explicitly asked to work in their environment;
- otherwise, with the user's permission, as a fallback when a headless environment can't be set up.

Never inject OS-level input (XTest and alike) into the user's session without permission: it moves their cursor, types
into whatever window has focus and may make the desktop ask to allow remote control.

aui-remote itself does not set up environments: it only talks to AUI applications. How an application behaves on a
particular desktop is a matter of AUI. A headless environment that worked on Linux with GNOME (mutter 50) and the X11
backend of AUI:

```bash
# if dbus-daemon is available, give the compositor a session bus of its own, so it doesn't touch services and portals
# of the user's session:
#   export DBUS_SESSION_BUS_ADDRESS=$(dbus-daemon --session --fork --print-address=1)
mutter --headless --wayland --virtual-monitor 1920x1080 --wayland-display aui-agent > mutter.log 2>&1 &
MUTTER_PID=$!
# the log says "Using public X11 display :N"; Xwayland's cookie is the newest of these files
XAUTH=$(ls -t /run/user/$(id -u)/.mutter-Xwaylandauth.* | head -1)

# X11 backend (the default one of AUI)
env -u WAYLAND_DISPLAY DISPLAY=:N XAUTHORITY=$XAUTH aui-remote cli launch -- ./app
# GTK backend (not tried yet)
env WAYLAND_DISPLAY=aui-agent AUI_PA=adw1 aui-remote cli launch -- ./app

kill $MUTTER_PID   # by pid: the user's desktop is mutter too, never kill by name
```

- The input of `click`, `type` and the rest is injected into the application by the agent, so it works there as
  anywhere else. The window manager is real: modality, focus and size hints behave as on the user's desktop.
- `echo "Xft.dpi: 144" | xrdb -merge` (with `DISPLAY` and `XAUTHORITY` of that display) gives the X11 backend a scale of
  1.5, to check how the app looks on a HiDPI screen.
- To debug how AUI itself interacts with the desktop (pointer grabs, size hints, modality), the input has to go through
  the OS instead: XTest on that display, i.e. a small Xlib program with `XTestFakeMotionEvent`,
  `XTestFakeButtonEvent`, `XTestFakeKeyEvent`, and `XGetImage` for screenshots of the actual window. There it stays
  inside the virtual monitor. Windows are placed at the center and overlap each other: check what is under the point
  (`XQueryPointer`) and activate the window (`_NET_ACTIVE_WINDOW`) before clicking.
- XTest does not reach Wayland windows (the GTK backend). In a GNOME session (the user's one, with their permission),
  OS-level input and screenshots of a monitor go through the D-Bus API of mutter:
  `org.gnome.Mutter.RemoteDesktop.CreateSession`, then `org.gnome.Mutter.ScreenCast.CreateSession` with
  `remote-desktop-session-id` and `RecordMonitor`, `Start`; `NotifyPointerMotionAbsolute` (monitor pixels),
  `NotifyPointerButton` (`BTN_LEFT` = 0x110), `NotifyKeyboardKeysym`; the stream is captured with
  `gst-launch-1.0 pipewiresrc path=<node of PipeWireStreamAdded>`. The session closes with the D-Bus connection that
  created it, so all the calls have to come from one process (Python with `gi`, not separate `gdbus` calls).
- With no headless environment at hand (macOS, Windows, other desktops), set one up yourself if you can and the user
  agrees (a VM, a separate session), or fall back to the user's environment with their permission.

**Starting**

- Start the application with `cli launch`, not by setting `AUI_REMOTE_KEY` yourself: `launch` stores the key it passes,
  a key you invent is unknown to the client and every command fails with "no longer trusts this client".
- The user often has their own instance of the same app running. Run `cli list` first and pass `--pid` (taken from the
  output of `launch`) to every command. Write it out in each command instead of keeping `--pid N` in one shell variable:
  zsh does not split variables, so `$R cli windows $P` sends `--pid 123` as a single argument.
- `stop` what you launched when you are done; leftovers pile up in `list`.
- If the app exits right away, `launch` returns `stderrTail`; read it before rebuilding anything.

**Looking**

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

**Acting**

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

**Debugging aui-remote with aui-remote**

In a Debug build of AUI itself (`AUI_REMOTE_SELF_DEBUG`, on by default there; never in builds that AUI.Boot makes for a
consumer, nor in release packages) the GUI links `aui::dev_extras` and starts the remote agent, so the CLI can drive it
like any other app:

```bash
./build/bin/aui-remote cli launch -- $PWD/build/bin/aui-remote   # prints the pid of the GUI
./build/bin/aui-remote cli screenshot --pid <pid>
```

On macOS the executable is `build/bin/aui-remote.app/Contents/MacOS/aui-remote`.

Only the GUI starts the agent, CLI invocations don't, and the GUI does not list itself among the connections. The GUI
shows the frames of the app it inspects inside its own window, so a screenshot of the GUI shows both; compare it with a
screenshot of the inspected app taken directly.

**Known gaps**

- Native dialogs (file pickers, message boxes) are drawn by the OS, not by AUI: they are absent from `tree` and
  screenshots and don't take injected input. An agent that opens one is stuck until a human closes it. Avoid them in
  the flows an agent drives, or expose the action as a function (`ARemoteDebug::registerFunction`) that takes the path
  directly.

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
`assets/skill/SKILL.md`, with `{{EXE}}` replaced by the path to this executable.
