# TCP client and server

This page shows how to talk over TCP with [ATcpSocket] and [ATcpServerSocket]. For UDP, see [network-udp].

## Connecting to a server

[ATcpSocket] connects in its constructor and blocks until the connection is established. The destination is an
[AInet4Address], which can be built from a hostname or an IP address together with a port.

```cpp
try {
    ATcpSocket socket(AInet4Address::fromString("example.com", 80));
    // connected
} catch (const AIOException& e) {
    ALogger::err("Net") << "Can't connect: " << e;
}
```

Both name resolution and connection failures throw [AIOException] (more precisely, its subclass `SocketException`).

## Reading and writing

The socket is an [IInputStream] and an [IOutputStream], so it can be passed anywhere a stream is expected. For example,
[json-parsing] mentions that `AJson::fromStream` reads from sockets.

```cpp
const AString request = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
auto utf8 = request.toStdString();
socket.write(utf8.data(), utf8.size());

char buffer[1024];
while (size_t n = socket.read(buffer, sizeof(buffer))) {
    // process n bytes
}
```

`read` returns 0 when the other side closes the connection. TCP does not preserve message boundaries: a single `read`
may return a part of what the peer wrote with one `write`, or several writes glued together. If you need exactly `N`
bytes, use `readExact`; for variable-sized messages, send the size first.

!!! note

    Socket calls block the calling thread. Do not use sockets directly in the UI thread; run them in a background
    thread (see `AUI_THREADPOOL`) and post the result back to the UI thread.

## Writing a server

[ATcpServerSocket] binds a port on construction. Each `accept()` call blocks until a client connects and returns a new
[ATcpSocket] for it.

```cpp
AAsyncHolder clients;
ATcpServerSocket server(8080);
for (;;) {
    _<ATcpSocket> client = server.accept();
    clients << AUI_THREADPOOL {
        try {
            char buffer[1024];
            while (size_t n = client->read(buffer, sizeof(buffer))) {
                client->write(buffer, n); // echo
            }
        } catch (const AIOException& e) {
            ALogger::warn("Net") << "Client dropped: " << e;
        }
    };
}
```

Every client is served by a task in the thread pool, which is kept alive by `AAsyncHolder`, so a slow client does not prevent others from connecting. The loop above
never ends; in a real application, run it in a dedicated thread and interrupt that thread to stop the server
(interrupting a thread blocked in a socket call makes it throw `AThread::Interrupted`).

## Closing

A socket is closed when it is destroyed. To close it earlier (e.g. to wake up a thread blocked in `read`), call
`close()`.

## Surprises

- Only IPv4 is supported.
