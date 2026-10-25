# Agentic development

An AI coding agent can run the whole loop on an AUI application: build, launch, look at the window, click and type, compare
the result and fix the code. This works through the remote agent of `aui::dev_extras`, an in-process JSON-RPC 2.0 server
on a WebSocket.

!!! warning

    The agent lets a client click, type and read what is on the screen. Link `aui::dev_extras` to development builds only
    and never ship it to users.

## Enabling

```cmake
auib_import(aui https://github.com/aui-framework/aui COMPONENTS core views dev_extras VERSION ${AUI_VERSION})
aui_link(${PROJECT_NAME} PRIVATE aui::dev_extras)   # development builds only
```

Run the application with `AUI_REMOTE=1`. The agent starts by itself, listens on `127.0.0.1` at a random port and writes
`~/.aui/remote/<pid>.json` (readable by the owner only) with the port and the token. To start it from code, call
[ARemoteAgent::start].

A client must present the token (`ws://127.0.0.1:<port>/?token=<token>` or `Authorization: Bearer`), and connections
carrying an `Origin` header (web pages) are rejected.

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
| `diag.log`                                            | recent log entries                                             |
| `fn.list`, `fn.call`                                  | application functions, see below                               |

Views are addressed by refs (`v<generation>:<id>`). A ref stops working when the view dies or the tree was re-listed;
the agent answers `STALE_REF` (-32002) and the client should repeat `view.find`.

Application specific functions help to reach states that are tedious to click through:

```cpp
ARemoteAgent::start()->registerFunction("openDocument", "arguments: { path: string }", [](const AJson& args) {
    openDocument(args["path"].asString());
    AFuture<AJson> result;
    result.supplyValue(AJson { { "ok", true } });
    return result;
});
```

## Using with an agent

The [aui-remote](https://github.com/aui-framework/remote) tool wraps the protocol into a command line that an agent can
call; its GUI installs a skill that teaches the agent the loop. Anything that speaks WebSocket works as well.
