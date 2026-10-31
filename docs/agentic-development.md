# Agentic development

An AI coding agent can run the whole loop on an AUI application: build, launch, look at the window, click and type, compare
the result and fix the code. This works through the remote agent of `aui::dev_extras`, an in-process JSON-RPC 2.0 server
on a WebSocket.

!!! warning

    The agent lets a client click, type and read what is on the screen. Link `aui::dev_extras` to development builds only
    and never ship it to users.

## Enabling

Remote debugging is opt-in: linking `aui::dev_extras` does nothing by itself, the application has to start
[ARemoteDebug] from its code. Link the module to development builds only:

```cmake
auib_import(aui https://github.com/aui-framework/aui COMPONENTS core views dev_extras VERSION ${AUI_VERSION})
```

<!-- aui:snippet examples/ui/remote_debug/CMakeLists.txt aui_link_dev_extras -->

Then start the agent in `AUI_ENTRY`. `AUI_DEBUG` keeps it out of release builds and `__has_include` keeps the code
compiling when `aui::dev_extras` is not linked:

<!-- aui:snippet examples/ui/remote_debug/src/main.cpp ARemoteDebug_include -->
<!-- aui:snippet examples/ui/remote_debug/src/main.cpp ARemoteDebug_start -->

The condition is yours to write: a command line flag, a setting or a debug menu item work as well. The code is the same
on every platform, including Windows, Android and iOS, where there is no convenient way to set an environment
variable. The agent listens on `127.0.0.1` at a random port (set `Options::port` to pin it) and writes
`~/.aui/remote/<pid>.json` with the port and the fingerprint of its certificate. On Android and iOS, where the
discovery file is out of reach, set `Options::writeDiscoveryFile` to `false`, pick a fixed port and forward it
(`adb forward tcp:<port> tcp:<port>`).

The complete application is `examples/ui/remote_debug`.

## Encryption

The agent speaks `wss://` only; there is no plain `ws://` mode. It generates a self-signed certificate on every start
and publishes its SHA-256 fingerprint as `certSha256` in the discovery file. A client must accept the server only when
the certificate matches that fingerprint: the discovery file (readable by its owner only) is the trusted channel, so
the traffic, including the key and the screenshots' paths, can't be read or altered by whoever sniffs the network.
[AWebsocket] does it with `Options::expectedFingerprint`.

[AWebsocketServer] with `Config::tls` and [ATlsSession] from `aui::crypt` are the building blocks.

## Pairing

A client that connects for the first time has to be paired, so that nothing on the machine can drive the application
behind your back:

1. The client calls `pair.begin`. The application writes a 6-digit code to the log and shows it in an overlay over its
   window.
2. A human types the code into the client (the GUI of `aui-remote`, or tells it to the agent that uses its CLI).
3. The client calls `pair.confirm` with `{ code, key }`, where `key` is a random string the client has just generated.

Pairing is a one-time procedure. The application stores a SHA-256 of the key (not the key itself) next to the path of its
executable, and the client stores the key the same way, so from now on the client connects with
`wss://127.0.0.1:<port>/?key=<key>` and needs no code, even after the application is restarted.
A code is valid for 2 minutes and 3 attempts (a refused `pair.confirm` reports `data.attemptsLeft`); after a few wasted codes `pair.begin` is refused (`RATE_LIMITED`, -32007)
for a period that doubles every time. Until paired, a connection can call nothing but `pair.begin` and
`pair.confirm`; connections with an `Origin` header (web pages) are rejected altogether.

To check every scenario while debugging, set `Options::pairing` (or the environment variable `AUI_REMOTE_PAIRING`,
read by `ARemoteDebug::Options::fromEnvironment()`):

| Value      | Behavior                                                                          |
|------------|-----------------------------------------------------------------------------------|
| `remember` | the default; the client is paired once                                            |
| `session`  | the client is trusted until the application exits; after a restart the code is asked again |
| `always`   | the code is asked on every connection, even from a client with a known key; `aui-remote cli` opens a connection per command, so only its GUI can work with it |

Where the hashes are kept is `Options::trustedKeysFile`: by default `~/.aui/remote/trusted` on desktop and the
application data folder on Android and iOS. On Emscripten nothing survives a reload, so it behaves like `session`.

When the client launches the application itself, it passes a key in the environment variable `AUI_REMOTE_KEY`, and no
pairing is needed. `ARemoteDebug::Options::fromEnvironment()` reads it, as in the example above.

### Keys of aui-remote

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

## Protocol

Text frames, JSON-RPC 2.0, protocol version 1. Errors carry `data.hint` with a suggestion for the next step.

| Method                                                | Purpose                                                        |
|-------------------------------------------------------|----------------------------------------------------------------|
| `session.hello`                                       | protocol version, pid and the list of methods                  |
| `window.list`                                         | windows and their ids                                          |
| `window.resize`                                       | resizes a window (`width`, `height`), answers when the layout is done |
| `window.setScale`                                     | the DPI scale on top of the system one (`factor` or `reset`)   |
| `view.tree`, `view.find`, `view.properties`           | inspect views; `view.find` filters by `type`, `text`, `assName`|
| `view.hitTest`, `view.interactive`                    | the views under a point; controls, text fields and views with a `clicked` connection |
| `view.scrollIntoView`                                 | scrolls the scroll areas around a view so that it is visible   |
| `screenshot`                                          | saves a PNG to the temp folder and returns its path; `ref` crops to a view, `annotate` numbers the interactive views |
| `input.pointer`                                       | `click`, `doubleClick`, `press`, `release`, `move`, `drag`, `scroll`, `longPress`, `fling` |
| `input.touch`                                         | several fingers at once: `pinch`, `rotate` or arbitrary tracks |
| `input.gesture`                                       | a long press gesture alone, for clients that hold the finger themselves |
| `input.type`, `input.key`                             | typed text and key presses with modifiers; they go to the window of `ref`, or `windowId` |
| `wait.idle`, `wait.view`                              | wait until nothing is happening / until a view appears or is gone |
| `frames.subscribe`, `frames.unsubscribe`, `frames.ack` | a stream of window frames with the view tree, see below     |
| `diag.log`                                            | recent log entries                                             |
| `fn.list`, `fn.call`                                  | application functions, see below                               |

`input.pointer` with `move` only moves the mouse, which is how to get a hover state without a click. The `pointer` param
selects the device: `mouse` or `finger`, which delivers the same actions as touch events of finger 0 (`button` is
ignored then). Without it, the pointer is the one native to the platform: `finger` on Android and iOS, `mouse` elsewhere;
`session.hello` and the discovery file report it as `defaultPointer`.

`click` holds the button for 50 ms before releasing it, and the call returns only after the release was delivered.

Touch gestures always use fingers, whatever `pointer` says, and the call returns once the last finger is lifted:

- `longPress` holds finger 0 for `durationMs` (600 by default), then delivers the long press gesture and lifts the
  finger. AUI has no long press timer of its own: on Android the gesture comes from the system, so the agent sends it
  the same way. The result has `handled`; a handled long press is not followed by a click.
- `fling` moves finger 0 by `dx`, `dy` with `velocity` px/s (2000 by default), a move per 16 ms, and lifts it, so the
  kinetic scroll of [AScrollArea] continues on its own; `wait.idle` waits for it to stop. AUI takes the velocity from
  the distance between the last moves, so a way shorter than three steps (`3 * velocity / 60`) gives a slower fling;
  the result reports the `velocity` delivered.
- `input.touch` plays `fingers: [{ finger, points: [{ x, y, t }] }]` (`t` in ms from the start, up to 30 s; between the
  points a finger moves smoothly), or a `gesture` around `ref` or `x`, `y`: `pinch` (`from`, `to`: the distance
  between two fingers, `angle` of the line between them) or `rotate` (`degrees`, `distance` between the fingers), both
  over `durationMs` (300 by default). AUI handles two-finger gestures in [A2FingerTransformArea] (pan, scale, rotation);
  other views see the fingers as separate pointers.
- `input.gesture` with `gesture: "longPress"` delivers only the gesture at the point. The `aui-remote` GUI sends it
  when a finger has been held still for 500 ms in T mode.

`screenshot` with `annotate: true` draws a numbered box over every interactive view that can be seen (set-of-mark) and
returns `marks: [{label, ref, role, text, rect}]`, so the numbers on the picture lead straight to refs for
`input.pointer`. The views are those of `view.interactive`, minus the ones inside an already marked view (the label of a
button); `rect` is in logical pixels of the window. The refs are of a new generation, like after `view.interactive`. The
marks are drawn into the saved image only: the window and the frame stream don't show them.

`window.resize` returns once the size has stayed the same for 100 ms with no pending layout, so a screenshot taken right
after it shows the new size. A window is never smaller than its contents need nor larger than 16384 pixels per axis, and
the window manager may adjust the size too; in that case the result also has `requestedWidth`, `requestedHeight`,
`minWidth`, `minHeight`, `maxWidth` and `maxHeight`.

`window.setScale` does what the Scale slider of the in-process devtools does: it multiplies the DPI ratio of the window
by `factor` (0.25..4, `ASurface::setScalingParams`), so the contents are laid out as on a screen with a different pixel
density while the window keeps its size in pixels, unless the contents no longer fit: then the window grows to their
minimum size and stays like that after the scale goes back. `reset: true` returns to the system scale. It answers when the layout
has settled, with `scalingFactor` and the resulting `dpi`. `window.list` and every `frames.frame` report
`scalingFactor` too.

For three seconds after every command, each window shows a pink "this AUI application is being debugged/controlled
remotely" banner at its top edge. Hover and click also leave a pink pointer on the screen for the same time (an arrow
for the mouse, a circle for a finger, plus a ripple on click), so someone sitting in front of the application can see
what the agent is doing. Between `press` and `release` both stay on the screen. Both are drawn by `aui.dev_extras` at the end of the
frame ([AWindow::postRender]) and never appear in screenshots: `screenshot` renders a clean frame first.

Views are addressed by refs (`v<generation>:<id>`). A ref stops working when the view dies or the tree was re-listed;
the agent answers `STALE_REF` (-32002) and the client should repeat `view.find`.

## Frame stream

An interactive client (i.e., a remote debugger window) doesn't poll `screenshot`. It sends `frames.subscribe` with
`{ windowId?, scale?, quality? }` (`scale` in (0, 1], default 0.5; JPEG `quality` 1..100, default 70) and then the
application pushes a `frames.frame` notification (a JSON-RPC message without `id`) every time the window has finished
rendering a frame. The first one comes right after subscribing. `frames.unsubscribe` stops the stream, so does closing
the connection. Subscribing again on the same connection replaces the parameters and restarts with a full tree.

```json
{ "jsonrpc": "2.0", "method": "frames.frame", "params": {
    "seq": 7, "width": 400, "height": 300, "windowWidth": 800, "windowHeight": 600,
    "jpeg": "<base64>",
    "tree": { "base": 5, "root": "v1:1", "upsert": { "v1:9": { "type": "ALabel", "text": "Hi", "x": 0, "y": 0,
                                                              "w": 40, "h": 20, "absX": 10, "absY": 10,
                                                              "visible": true, "children": [] } },
              "remove": ["v1:12"] } } }
```

`width` and `height` are the size of the encoded image in pixels, `windowWidth` and `windowHeight` are the logical size
of the window. The tree is a flat map of refs to nodes (`type`, `text` and `name` (ASS names) if there are any, `layout` of
a container, geometry relative to the parent and to the window, `padding` and `margin` as `[left, top, right, bottom]`
unless they are zero, `visible`, `children` as refs), the same refs as in `view.tree`.

The tree is not sent in full every time: it is a difference against the last tree the client has confirmed with
`frames.ack` (params `{ seq }`), `base` being the `seq` of that frame, or `null` for a full tree. `upsert` holds every
node that is new or differs from the base (the whole node replaces the old one), `remove` holds refs of vanished nodes.
A client applies the diff to the tree of frame `base` and then acknowledges the frame. Since frames can be dropped, a
diff never depends on frames that were not acknowledged.

There is at most one frame in flight per connection. While one is being sent, newer renders are not queued; when the
send completes, a frame with the current state is sent. The remote control overlay is not in the frames.

!!! note

    `frames.ack` does not show the overlay banner, unlike other commands: otherwise acknowledging a frame would
    render the next one, forever.

Application specific functions help to reach states that are tedious to click through:

```cpp
ARemoteDebug::start()->registerFunction("openDocument", "arguments: { path: string }", [](const AJson& args) {
    openDocument(args["path"].asString());
    return AFuture<AJson>::fromValue(AJson { { "ok", true } });
});
```

## Using with an agent

`aui-remote` (module `aui.remote`) wraps the protocol into a command line that an agent can call; its GUI installs a
skill that teaches the agent the loop. It is built for the host along with AUI and comes in `bin/` of AUI
distributions, next to `aui.toolbox`. In a consumer project, find it through the `aui` entry of
`build/package-lock.json`, which AUI.Boot writes on configure:

- `as_subdirectory` is `false`: the tool is `<install_dir>/bin/aui-remote`;
- `as_subdirectory` is `true` (`-DAUIB_AUI_AS=ON`, see [configure flags](aui-configure-flags.md)): AUI is a part of the
  consumer's build, so build it with `cmake --build build -t aui.remote` and take `build/bin/aui-remote`.

Anything that speaks WebSocket works as well.

The commands are listed by `aui-remote cli help`; the skill contains the same table and the instructions for the agent.

## Running in a headless environment

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

## Debugging aui-remote with aui-remote

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
