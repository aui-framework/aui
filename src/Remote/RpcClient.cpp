#include "RpcClient.h"

#include <random>

#if !AUI_PLATFORM_WIN
#include <signal.h>
#endif

#include <AUI/Crypt/ATlsSession.h>
#include <AUI/Curl/Detail/WebsocketFrame.h>
#include <AUI/Network/AInet4Address.h>
#include <AUI/Network/ATcpSocket.h>

using namespace aui::websocket;
using namespace std::chrono_literals;

namespace aui_remote {

AJson RpcError::toJson() const {
    AJson result = AJson::Object {};
    result["code"] = mCode;
    result["message"] = AString(getMessage());
    if (!mData.isEmpty() && !mData.isNull()) {
        result["data"] = mData;
    }
    return result;
}

RpcClient RpcClient::forApp(const AppInfo& app, AString key) {
    if (!app.isSupported()) {
        throw RpcError(
            rpc_code::CLIENT_ERROR, "{} does not offer a pinned wss connection"_format(app.displayName()),
            AJson { { "hint", "the discovery file needs scheme \"wss\" and certSha256; rebuild the application with a "
                              "newer aui::dev_extras" } });
    }
    return RpcClient(app.port, app.certSha256, std::move(key));
}

AString RpcClient::target() const {
    AString result = "/";
    if (!mKey.empty()) {
        result += "?key=" + mKey;
    }
    return result;
}

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

/// One blocking connection: TLS pinned to a fingerprint, HTTP upgrade, masked frames.
class Connection {
public:
    Connection(std::uint16_t port, const AString& certSha256, const AString& target,
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

    ~Connection() {
        try {
            send(Opcode::CLOSE, {});
        } catch (...) {
        }
        if (mTls) {
            mTls->close();
        }
    }

    void sendText(const AString& text) { send(Opcode::TEXT, AByteBufferView(text.data(), text.size())); }

    /// @return the next text message
    AString readText() {
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
            const auto left = mDeadline - std::chrono::steady_clock::now();
            if (left <= 0ms || !mSocket.waitForRead(std::chrono::duration_cast<std::chrono::milliseconds>(left))) {
                throw RpcError(
                    rpc_code::CLIENT_ERROR, "timeout waiting for the application",
                    AJson { { "hint", "the application is busy or hung; try again or raise the timeout" } });
            }
            char buf[4096];
            const auto n = mTls->read(buf, sizeof(buf));
            if (n == 0) {
                throw RpcError(rpc_code::CLIENT_ERROR, "the application closed the connection");
            }
            mDecoder.feed(AByteBufferView(buf, n));
        }
    }

private:
    ATcpSocket mSocket;
    _<ATlsSession> mTls;
    FrameDecoder mDecoder;
    std::chrono::steady_clock::time_point mDeadline;

    void send(Opcode opcode, AByteBufferView payload) {
        std::random_device device;
        Mask mask;
        for (auto& m : mask) {
            m = std::uint8_t(device());
        }
        auto frame = encodeFrame(opcode, payload, true, mask);
        mTls->write(frame.data(), frame.size());
    }
};

}   // namespace

AVector<AJson> RpcClient::callSequence(const AVector<RpcRequest>& requests, std::chrono::milliseconds timeout) const {
#if !AUI_PLATFORM_WIN
    // a vanished peer must be an error of the call, not the death of the process
    ::signal(SIGPIPE, SIG_IGN);
#endif
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    AVector<AJson> results;
    try {
        Connection connection(mPort, mCertSha256, target(), deadline);
        for (std::size_t i = 0; i < requests.size(); ++i) {
            const auto& request = requests[i];
            AJson json = AJson::Object {};
            json["jsonrpc"] = "2.0";
            json["id"] = int(i + 1);
            json["method"] = request.method;
            if (!request.params.isEmpty() && !request.params.isNull()) {
                json["params"] = request.params;
            }
            connection.sendText(AJson::toString(json));

            for (;;) {
                AJson answer;
                try {
                    answer = AJson::fromString(connection.readText());
                } catch (const AJsonException&) {
                    continue;
                }
                if (!answer.isObject()) {
                    continue;
                }
                auto id = answer.containsOpt("id");
                if (!id || !id->isInt() || id->asInt() != int(i + 1)) {
                    continue;   // notification or someone else's answer
                }
                if (auto e = answer.containsOpt("error"); e && e->isObject()) {
                    AJson data;
                    if (auto d = e->containsOpt("data")) {
                        data = *d;
                    }
                    throw RpcError(
                        e->containsOpt("code") ? (*e)["code"].asInt() : rpc_code::CLIENT_ERROR,
                        e->containsOpt("message") ? (*e)["message"].asString() : AString("unknown error"),
                        std::move(data));
                }
                results << (answer.containsOpt("result") ? answer["result"] : AJson(nullptr));
                break;
            }
        }
    } catch (const RpcError&) {
        throw;
    } catch (const ProtocolError& e) {
        throw RpcError(rpc_code::CLIENT_ERROR, AString("protocol error: ") + e.what());
    } catch (const ATlsException& e) {
        throw RpcError(
            rpc_code::CLIENT_ERROR, "secure connection failed: " + e.getMessage(),
            AJson { { "hint", "the certificate does not match the discovery file; the application may have been "
                              "restarted or is not the one it claims to be" } });
    } catch (const AException& e) {
        throw RpcError(
            rpc_code::CLIENT_ERROR, "can't talk to the application: " + e.getMessage(),
            AJson { { "hint", "check that the application is running" } });
    }
    return results;
}

AJson RpcClient::call(AString method, AJson params, std::chrono::milliseconds timeout) const {
    return std::move(callSequence({ RpcRequest { std::move(method), std::move(params) } }, timeout).at(0));
}

}   // namespace aui_remote
