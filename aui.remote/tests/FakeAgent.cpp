#include "FakeAgent.h"

#include "network/RpcClient.h"

namespace aui_remote::test {

FakeAgent::FakeAgent(AString preAuthorizedKey) {
    if (!preAuthorizedKey.empty()) {
        mKeys.insert(preAuthorizedKey.toStdString());
    }
    AWebsocketServer::Config config;
    config.tls = mIdentity;
    config.onConnected = [this](_<AWebsocketServer::Connection> connection) {
        auto* c = connection.get();
        bool authorized = false;
        if (auto key = c->request().queryParameter("key")) {
            authorized = hasKey(*key);
        }
        {
            std::unique_lock lock(mMutex);
            mConnections.push_back(c);
        }
        c->onClosed = [this, c] {
            std::unique_lock lock(mMutex);
            std::erase(mSubscribers, c);
            std::erase(mConnections, c);
        };
        c->onMessage = [this, c, authorized](AByteBuffer message, bool) mutable {
            auto request = AJson::fromString(AString::fromUtf8(message));
            const bool isNotification = !request.containsOpt("id");
            AJson response = AJson::Object {};
            response["jsonrpc"] = "2.0";
            if (!isNotification) {
                response["id"] = request["id"];
            }
            try {
                response["result"] = handle(
                    request["method"].asString(),
                    request.containsOpt("params") ? request["params"] : AJson(AJson::Object {}), authorized, c);
            } catch (const RpcError& e) {
                response["error"] = e.toJson();
            }
            if (isNotification) {
                return;
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

AJson FakeAgent::handle(const AString& method, const AJson& params, bool& authorized,
                        AWebsocketServer::Connection* connection) {
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
        return AJson { { "paired", true }, { "trust", "persistent" } };
    }
    if (!authorized) {
        throw RpcError(rpc_code::UNAUTHORIZED, "unauthorized", AJson { { "hint", "pair first" } });
    }
    if (method == "session.hello") {
        return AJson { { "protocol", 1 } };
    }
    if (method == "window.list") {
        return AJson { { "windows", AJson::Array { AJson { { "windowId", "v1:1" }, { "title", "Fake" } } } } };
    }
    if (method == "frames.subscribe") {
        ++mSubscribeCalls;
        mLastSubscribe = params;
        if (std::find(mSubscribers.begin(), mSubscribers.end(), connection) == mSubscribers.end()) {
            mSubscribers.push_back(connection);
        }
        return AJson { { "subscribed", true } };
    }
    if (method == "frames.unsubscribe") {
        std::erase(mSubscribers, connection);
        return AJson::Object {};
    }
    if (method == "frames.ack") {
        mAcks << params["seq"].asInt();
        return AJson::Object {};
    }
    if (method.startsWith("input.")) {
        mInputCalls << Call { method, params };
        return AJson::Object {};
    }
    if (method == "view.find") {
        return AJson { { "echo", params } };
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

namespace aui_remote::test {

int FakeAgent::pushFrame(const AJson& params) {
    AJson notification = AJson::Object {};
    notification["jsonrpc"] = "2.0";
    notification["method"] = "frames.frame";
    notification["params"] = params;
    const auto text = AJson::toString(notification);
    std::unique_lock lock(mMutex);
    for (auto* c : mSubscribers) {
        c->sendText(text);
    }
    return int(mSubscribers.size());
}

int FakeAgent::subscribers() const {
    std::unique_lock lock(mMutex);
    return int(mSubscribers.size());
}

int FakeAgent::subscribeCalls() const {
    std::unique_lock lock(mMutex);
    return mSubscribeCalls;
}

AJson FakeAgent::lastSubscribeParams() const {
    std::unique_lock lock(mMutex);
    return mLastSubscribe;
}

AVector<int> FakeAgent::acks() const {
    std::unique_lock lock(mMutex);
    return mAcks;
}

AVector<FakeAgent::Call> FakeAgent::inputCalls() const {
    std::unique_lock lock(mMutex);
    return mInputCalls;
}

void FakeAgent::dropConnections() {
    std::vector<AWebsocketServer::Connection*> copy;
    {
        std::unique_lock lock(mMutex);
        copy = mConnections;
    }
    for (auto* c : copy) {
        c->close();
    }
}

}   // namespace aui_remote::test
