#include "WsConnection.h"

#include <random>

#include <AUI/Network/AInet4Address.h>

using namespace aui::websocket;
using namespace std::chrono_literals;

namespace aui_remote {

namespace {

AByteBuffer fromHex(const AString& hex) {
    AByteBuffer result;
    auto digit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        const int hi = digit(hex[i]);
        const int lo = digit(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            throw RpcError(rpc_code::CLIENT_ERROR, "certSha256 is not a hex string");
        }
        const char byte = char(hi * 16 + lo);
        result.write(&byte, 1);
    }
    return result;
}


}   // namespace

WsConnection::WsConnection(std::uint16_t port, const AString& certSha256, const AString& target,
                            std::chrono::steady_clock::time_point deadline)
  : mSocket(AInet4Address::fromString("127.0.0.1", port)), mDecoder(false, 1 << 28), mDeadline(deadline) {
    mSocket.setTimeout(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()));
    ATlsTransport transport {
        [this](std::chrono::milliseconds t) { return mSocket.waitForRead(t); },
        [this](char* dst, std::size_t size) { return mSocket.read(dst, size); },
        [this](const char* src, std::size_t size) { mSocket.write(src, size); },
    };
    ATlsSession::ClientOptions options;
    options.expectedFingerprint = fromHex(certSha256);
    mTls = ATlsSession::connect(std::move(transport), options, 5s);

    std::random_device device;
    char nonce[16];
    for (auto& c : nonce) {
        c = char(device());
    }
    std::string request = "GET " + target.toStdString() +
                          " HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                          "Sec-WebSocket-Version: 13\r\nSec-WebSocket-Key: " +
                          AByteBuffer(nonce, sizeof(nonce)).toBase64String().toStdString() + "\r\n\r\n";
    mTls->write(request.data(), request.size());

    std::string head;
    char c;
    while (head.find("\r\n\r\n") == std::string::npos) {
        if (mTls->read(&c, 1) == 0) {
            break;
        }
        head += c;
    }
    if (head.rfind("HTTP/1.1 101", 0) != 0) {
        const auto line = head.substr(0, head.find("\r\n"));
        throw RpcError(
            rpc_code::CLIENT_ERROR, "the application refused the connection: " + AString::fromUtf8(line),
            AJson { { "hint", "the key is missing or wrong; run `aui-remote cli pair <pid>`" } });
    }
}

WsConnection::~WsConnection() {
    try {
        send(Opcode::CLOSE, {});
    } catch (...) {
    }
    if (mTls) {
        mTls->close();
    }
}

void WsConnection::sendText(const AString& text) { send(Opcode::TEXT, AByteBufferView(text.data(), text.size())); }

AString WsConnection::readText() {
    auto text = readTextUntil(mDeadline);
    if (!text) {
        throw RpcError(
            rpc_code::CLIENT_ERROR, "timeout waiting for the application",
            AJson { { "hint", "the application is busy or hung; try again or raise the timeout" } });
    }
    return std::move(*text);
}

std::optional<AString> WsConnection::readText(std::chrono::milliseconds timeout) {
    return readTextUntil(std::chrono::steady_clock::now() + timeout);
}

std::optional<AString> WsConnection::readTextUntil(std::chrono::steady_clock::time_point deadline) {
    for (;;) {
        while (auto frame = mDecoder.next()) {
            switch (frame->opcode) {
                case Opcode::TEXT:
                case Opcode::BINARY:
                    return AString::fromUtf8(frame->payload);
                case Opcode::PING:
                    send(Opcode::PONG, frame->payload);
                    break;
                case Opcode::CLOSE:
                    throw RpcError(rpc_code::CLIENT_ERROR, "the application closed the connection");
                default:
                    break;
            }
        }
        const auto left = deadline - std::chrono::steady_clock::now();
        if (left <= 0ms || !mSocket.waitForRead(std::chrono::duration_cast<std::chrono::milliseconds>(left))) {
            return std::nullopt;
        }
        char buf[1 << 15];   // at least a TLS record: what is left of it is invisible to poll()
        const auto n = mTls->read(buf, sizeof(buf));
        if (n == 0) {
            throw RpcError(rpc_code::CLIENT_ERROR, "the application closed the connection");
        }
        mDecoder.feed(AByteBufferView(buf, n));
    }
}

void WsConnection::send(Opcode opcode, AByteBufferView payload) {
    std::random_device device;
    Mask mask;
    for (auto& m : mask) {
        m = std::uint8_t(device());
    }
    auto frame = encodeFrame(opcode, payload, true, mask);
    std::unique_lock lock(mSendMutex);
    mTls->write(frame.data(), frame.size());
}

}   // namespace aui_remote
