#pragma once

#include <chrono>
#include <mutex>
#include <optional>

#include <AUI/Crypt/ATlsSession.h>
#include <AUI/Curl/Detail/WebsocketFrame.h>
#include <AUI/Network/ATcpSocket.h>

#include "RpcClient.h"

namespace aui_remote {

/**
 * @brief One blocking client connection: TLS pinned to a fingerprint, HTTP upgrade, masked WebSocket frames.
 * @details
 * One thread may read while others send (sending is serialized).
 */
class WsConnection {
public:
    /**
     * @throws RpcError if the application refuses the connection
     * @throws AException (ATlsException, AIOException...) on a transport failure, before `deadline`
     */
    WsConnection(std::uint16_t port, const AString& certSha256, const AString& target,
                 std::chrono::steady_clock::time_point deadline);
    ~WsConnection();

    void sendText(const AString& text);

    /**
     * @return the next text message
     * @throws RpcError on timeout (the deadline of the constructor) or when the connection is closed
     */
    AString readText();

    /**
     * @return the next text message; nullopt if there is none in `timeout`
     * @throws RpcError when the connection is closed
     */
    std::optional<AString> readText(std::chrono::milliseconds timeout);

private:
    ATcpSocket mSocket;
    _<ATlsSession> mTls;
    aui::websocket::FrameDecoder mDecoder;
    std::chrono::steady_clock::time_point mDeadline;
    std::mutex mSendMutex;

    std::optional<AString> readTextUntil(std::chrono::steady_clock::time_point deadline);
    void send(aui::websocket::Opcode opcode, AByteBufferView payload);
};

}   // namespace aui_remote
