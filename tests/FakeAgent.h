#pragma once

#include <atomic>
#include <mutex>
#include <set>

#include <AUI/Curl/AWebsocketServer.h>
#include <AUI/Json/AJson.h>

namespace aui_remote::test {

/**
 * @brief Stands in for the remote agent of an application: JSON-RPC over WebSocket with the pairing flow.
 * @details
 * Unauthorized connections may call only `pair.begin` and `pair.confirm`; the right code is 123456 and three wrong ones
 * void it. `window.list` and `view.tree` answer canned data, `fail` answers with an error.
 */
class FakeAgent {
public:
    static constexpr const char* CODE = "123456";

    explicit FakeAgent(AString preAuthorizedKey = {});

    [[nodiscard]] std::uint16_t port() const noexcept { return mServer->port(); }
    [[nodiscard]] bool hasKey(const AString& key) const;
    [[nodiscard]] int pairBeginCalls() const noexcept { return mPairBegin; }

private:
    std::unique_ptr<AWebsocketServer> mServer;
    mutable std::mutex mMutex;
    std::set<std::string> mKeys;
    std::atomic<int> mPairBegin = 0;
    bool mCodeActive = false;
    int mAttempts = 0;

    AJson handle(const AString& method, const AJson& params, bool& authorized);
};

}   // namespace aui_remote::test
