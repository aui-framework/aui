# UDP datagrams and ping

[AUdpSocket] sends and receives independent datagrams. Unlike [ATcpSocket], there is no connection: every call names
the peer explicitly, delivery is not guaranteed and datagrams may arrive reordered. In return, message boundaries are
preserved: one `write` is received by exactly one `read`.

## Receiver

Construct the socket with a port to listen on. `read` blocks until a datagram arrives, fills the buffer with its payload
and tells who sent it.

```cpp
AUdpSocket socket(9000);
for (;;) {
    AByteBuffer buffer;
    AInet4Address sender;
    socket.read(buffer, sender);
    ALogger::info("Net") << "Got " << buffer.size() << " bytes from " << sender.toString();
}
```

## Sender

The default constructor picks a random port, which is what a client usually wants. `write` takes an
`AByteBufferView` and the destination.

```cpp
AUdpSocket socket;
auto payload = AByteBuffer::fromString("ping");
socket.write(payload, AInet4Address::fromString("127.0.0.1", 9000));
```

A datagram must be smaller than 32768 bytes (the sending side asserts this). If your messages are bigger, split them
yourself or use [network-tcp].

Replying works the same way: pass the `sender` address obtained by `read` to `write`.

## Pinging a host

`AIcmp::ping` sends an ICMP echo request and returns an [AFuture] of the round trip duration. The future holds an
exception if the host did not answer within the timeout (5 seconds by default).

```cpp
try {
    auto rtt = *AIcmp::ping(AInet4Address::fromString("127.0.0.1"), std::chrono::seconds(1));
    ALogger::info("Net") << "Round trip: " << std::chrono::duration_cast<std::chrono::milliseconds>(rtt).count()
                         << "ms";
} catch (const AException& e) {
    ALogger::warn("Net") << "No reply: " << e;
}
```

See `aui.network/tests/PingTest.cpp` for more usages.
