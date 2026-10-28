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
Clients based on libcurl do not verify the peer, which is fine on the loopback but not a protection.

[AWebsocketServer] with `Config::tls` and [ATlsSession] from `aui::crypt` are the building blocks.

## Pairing

A client that connects for the first time has to be paired, so that nothing on the machine can drive the application
behind your back:

1. The client calls `pair.begin`. The application writes a 6-digit code to the log and shows it in an overlay over its
   window.
2. A human types the code into the client (the GUI of [aui-remote](https://github.com/aui-framework/remote), or tells it
   to the agent that uses its CLI).
3. The client calls `pair.confirm` with `{ code, key }`, where `key` is a random string the client has just generated.

Pairing is a one-time procedure. The application stores a SHA-256 of the key (not the key itself) next to the path of its
executable, and the client stores the key the same way, so from now on the client connects with
`wss://127.0.0.1:<port>/?key=<key>` and needs no code, even after the application is restarted.
A code is valid for 2 minutes and 3 attempts; after a few wasted codes `pair.begin` is refused (`RATE_LIMITED`, -32007)
for a period that doubles every time. Until paired, a connection can call nothing but `pair.begin` and
`pair.confirm`; connections with an `Origin` header (web pages) are rejected altogether.

To check every scenario while debugging, set `Options::pairing` (or the environment variable `AUI_REMOTE_PAIRING`,
read by [ARemoteDebug::Options::fromEnvironment]):

| Value      | Behavior                                                                          |
|------------|-----------------------------------------------------------------------------------|
| `remember` | the default; the client is paired once                                            |
| `session`  | the client is trusted until the application exits; after a restart the code is asked again |
| `always`   | the code is asked on every connection, even from a client with a known key        |

Where the hashes are kept is `Options::trustedKeysFile`: by default `~/.aui/remote/trusted` on desktop and the
application data folder on Android and iOS. On Emscripten nothing survives a reload, so it behaves like `session`.

When the client launches the application itself, it passes a key in the environment variable `AUI_REMOTE_KEY`, and no
pairing is needed. [ARemoteDebug::Options::fromEnvironment] reads it, as in the example above.

## Protocol

Text frames, JSON-RPC 2.0, protocol version 1. Errors carry `data.hint` with a suggestion for the next step.

| Method                                                | Purpose                                                        |
|-------------------------------------------------------|----------------------------------------------------------------|
| `session.hello`                                       | protocol version, pid and the list of methods                  |
| `window.list`                                         | windows and their ids                                          |
| `view.tree`, `view.find`, `view.properties`           | inspect views; `view.find` filters by `type`, `text`, `assName`|
| `view.hitTest`, `view.interactive`                    | the views under a point; clickable / typeable elements         |
| `screenshot`                                          | saves a PNG to the temp folder and returns its path; `ref` crops to a view |
| `input.pointer`                                       | `click`, `doubleClick`, `press`, `release`, `move`, `drag`, `scroll` |
| `input.type`, `input.key`                             | typed text and key presses with modifiers                      |
| `wait.idle`, `wait.view`                              | wait until nothing is happening / until a view appears or is gone |
| `frames.subscribe`, `frames.unsubscribe`, `frames.ack` | a stream of window frames with the view tree, see below     |
| `diag.log`                                            | recent log entries                                             |
| `fn.list`, `fn.call`                                  | application functions, see below                               |

`input.pointer` with `move` only moves the mouse, which is how to get a hover state without a click. The `pointer` param
selects the device: `mouse` (default) or `finger`, which delivers the same actions as touch events of finger 0 (`button`
is ignored then).

`click` holds the button for 50 ms before releasing it, and the call returns only after the release was delivered.

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
of the window. The tree is a flat map of refs to nodes (`type`, `text` and `name` (ASS names) if there are any, geometry
relative to the parent and to the window, `visible`, `children` as refs), the same refs as in `view.tree`.

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
    AFuture<AJson> result;
    result.supplyValue(AJson { { "ok", true } });
    return result;
});
```

## Using with an agent

The [aui-remote](https://github.com/aui-framework/remote) tool wraps the protocol into a command line that an agent can
call; its GUI installs a skill that teaches the agent the loop. Anything that speaks WebSocket works as well.
