#include "RpcClient.h"

#include <random>

#if !AUI_PLATFORM_WIN
#include <signal.h>
#endif

#include "WsConnection.h"

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

AVector<AJson> RpcClient::callSequence(const AVector<RpcRequest>& requests, std::chrono::milliseconds timeout) const {
#if !AUI_PLATFORM_WIN
    // a vanished peer must be an error of the call, not the death of the process
    ::signal(SIGPIPE, SIG_IGN);
#endif
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    AVector<AJson> results;
    try {
        WsConnection connection(mPort, mCertSha256, target(), deadline);
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
