#pragma once

#include <atomic>
#include <mutex>
#include <set>
#include <vector>

#include <AUI/Crypt/ATlsIdentity.h>
#include <AUI/Curl/AWebsocketServer.h>
#include <AUI/Json/AJson.h>

namespace aui_remote::test {

/**
 * @brief Stands in for the remote agent of an application: JSON-RPC over WebSocket with the pairing flow.
 * @details
 * Unauthorized connections may call only `pair.begin` and `pair.confirm`; the right code is 123456 and three wrong ones
 * void it. `window.list` and `view.tree` answer canned data, `fail` answers with an error.
 *
 * Frame push: `frames.subscribe` registers the connection, pushFrame() sends a `frames.frame` notification to the
 * subscribed connections, `frames.ack` and `input.*` calls are recorded. `view.find` echoes its params.
 */
class FakeAgent {
public:
    static constexpr const char* CODE = "123456";

    explicit FakeAgent(AString preAuthorizedKey = {});

    [[nodiscard]] std::uint16_t port() const noexcept { return mServer->port(); }
    /**
     * @return SHA-256 of the certificate of the agent, hex; what the discovery file carries
     */
    [[nodiscard]] AString certSha256() const { return mIdentity->fingerprintHex(); }

    [[nodiscard]] bool hasKey(const AString& key) const;
    [[nodiscard]] int pairBeginCalls() const noexcept { return mPairBegin; }

    struct Call {
        AString method;
        AJson params;
    };

    /**
     * @brief Sends a `frames.frame` notification with the params to every subscribed connection.
     * @return number of connections the frame was sent to
     */
    int pushFrame(const AJson& params);
    [[nodiscard]] int subscribers() const;
    [[nodiscard]] int subscribeCalls() const;
    [[nodiscard]] AJson lastSubscribeParams() const;
    [[nodiscard]] AVector<int> acks() const;
    /// `input.*` calls, in the order of arrival
    [[nodiscard]] AVector<Call> inputCalls() const;
    /// Closes every connection from the agent side.
    void dropConnections();

private:
    _<ATlsIdentity> mIdentity = ATlsIdentity::generateSelfSigned("aui-remote-test");
    std::unique_ptr<AWebsocketServer> mServer;
    mutable std::mutex mMutex;
    std::set<std::string> mKeys;
    std::atomic<int> mPairBegin = 0;
    bool mCodeActive = false;
    int mAttempts = 0;
    std::vector<AWebsocketServer::Connection*> mSubscribers;
    std::vector<AWebsocketServer::Connection*> mConnections;
    int mSubscribeCalls = 0;
    AJson mLastSubscribe;
    AVector<int> mAcks;
    AVector<Call> mInputCalls;

    AJson handle(const AString& method, const AJson& params, bool& authorized, AWebsocketServer::Connection* connection);
};

}   // namespace aui_remote::test
