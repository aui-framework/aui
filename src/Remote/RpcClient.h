#pragma once

#include <chrono>
#include <cstdint>
#include <utility>

#include <AUI/Common/AException.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Json/AJson.h>

namespace aui_remote {

/**
 * @brief JSON-RPC error answered by the agent.
 */
class RpcError : public AException {
public:
    RpcError(int code, AString message, AJson data = {})
      : AException(std::move(message)), mCode(code), mData(std::move(data)) {}

    [[nodiscard]] int code() const noexcept { return mCode; }
    [[nodiscard]] const AJson& data() const noexcept { return mData; }

    /**
     * @return `{ code, message, data }`
     */
    [[nodiscard]] AJson toJson() const;

private:
    int mCode;
    AJson mData;
};

/**
 * @brief Error codes of the protocol that the client reacts to.
 */
namespace rpc_code {
constexpr int UNAUTHORIZED = -32001;

/**
 * @brief Raised by the client itself, not the agent.
 */
constexpr int CLIENT_ERROR = -32099;
}   // namespace rpc_code

struct RpcRequest {
    AString method;
    AJson params;
};

/**
 * @brief Synchronous JSON-RPC 2.0 client over a short-lived WebSocket connection.
 * @details
 * Every call opens a connection, sends the requests one after another, waits for the answers and closes the
 * connection. The requests of one call share the connection.
 */
class RpcClient {
public:
    static constexpr std::chrono::milliseconds DEFAULT_TIMEOUT = std::chrono::seconds(15);

    /**
     * @param port port of the agent
     * @param key key to authorize the connection; empty to connect as unauthorized
     */
    RpcClient(std::uint16_t port, AString key) : mPort(port), mKey(std::move(key)) {}

    /**
     * @return `result` of the answer
     * @throws RpcError if the agent answers with an error, the connection is refused or lost, or the time is out
     */
    AJson call(AString method, AJson params = {}, std::chrono::milliseconds timeout = DEFAULT_TIMEOUT) const;

    /**
     * @return `result` of every request
     * @throws RpcError on the first failed request
     */
    AVector<AJson> callSequence(const AVector<RpcRequest>& requests,
                                std::chrono::milliseconds timeout = DEFAULT_TIMEOUT) const;

    /**
     * @return the WebSocket URL used to connect, with the key as the query
     */
    [[nodiscard]] AString url() const;

private:
    std::uint16_t mPort;
    AString mKey;
};

}   // namespace aui_remote
