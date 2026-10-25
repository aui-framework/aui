#include "FakeAgent.h"

#include "Remote/RpcClient.h"

namespace aui_remote::test {

FakeAgent::FakeAgent(AString preAuthorizedKey) {
    if (!preAuthorizedKey.empty()) {
        mKeys.insert(preAuthorizedKey.toStdString());
    }
    AWebsocketServer::Config config;
    config.onConnected = [this](_<AWebsocketServer::Connection> connection) {
        auto* c = connection.get();
        bool authorized = false;
        if (auto key = c->request().queryParameter("key")) {
            authorized = hasKey(*key);
        }
        c->onMessage = [this, c, authorized](AByteBuffer message, bool) mutable {
            auto request = AJson::fromString(AString::fromUtf8(message));
            AJson response = AJson::Object {};
            response["jsonrpc"] = "2.0";
            response["id"] = request["id"];
            try {
                response["result"] = handle(
                    request["method"].asString(),
                    request.containsOpt("params") ? request["params"] : AJson(AJson::Object {}), authorized);
            } catch (const RpcError& e) {
                response["error"] = e.toJson();
            }
            c->sendText(AJson::toString(response));
        };
    };
    mServer = std::make_unique<AWebsocketServer>(std::move(config));
    mServer->start();
}

bool FakeAgent::hasKey(const AString& key) const {
    std::unique_lock lock(mMutex);
    return mKeys.contains(key.toStdString());
}

AJson FakeAgent::handle(const AString& method, const AJson& params, bool& authorized) {
    std::unique_lock lock(mMutex);
    if (method == "pair.begin") {
        ++mPairBegin;
        mCodeActive = true;
        mAttempts = 0;
        return AJson { { "pending", true } };
    }
    if (method == "pair.confirm") {
        if (!mCodeActive) {
            throw RpcError(rpc_code::UNAUTHORIZED, "no pairing in progress", AJson { { "hint", "call pair.begin" } });
        }
        if (params["code"].asString() != CODE) {
            ++mAttempts;
            if (mAttempts >= 3) {
                mCodeActive = false;
            }
            throw RpcError(
                rpc_code::UNAUTHORIZED, "wrong code",
                AJson { { "attemptsLeft", std::max(0, 3 - mAttempts) }, { "hint", "read the code again" } });
        }
        mKeys.insert(params["key"].asString().toStdString());
        mCodeActive = false;
        authorized = true;
        return AJson { { "paired", true } };
    }
    if (!authorized) {
        throw RpcError(rpc_code::UNAUTHORIZED, "unauthorized", AJson { { "hint", "pair first" } });
    }
    if (method == "window.list") {
        return AJson { { "windows", AJson::Array { AJson { { "windowId", "v1:1" }, { "title", "Fake" } } } } };
    }
    if (method == "view.tree") {
        return AJson { { "ref", "v1:1" }, { "type", "AWindow" }, { "echoDepth", params.containsOpt("depth") ? params["depth"] : AJson(-1) } };
    }
    if (method == "fail") {
        throw RpcError(-32002, "STALE_REF", AJson { { "hint", "find again" } });
    }
    throw RpcError(-32601, "method not found: " + method);
}

}   // namespace aui_remote::test
