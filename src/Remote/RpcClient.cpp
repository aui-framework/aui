#include "RpcClient.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#include <AUI/Curl/AWebsocket.h>

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

AString RpcClient::url() const {
    AString result = "ws://127.0.0.1:{}/"_format(mPort);
    if (!mKey.empty()) {
        result += "?key=" + mKey;
    }
    return result;
}

namespace {

struct Session {
    AVector<RpcRequest> requests;
    AVector<AJson> results;
    std::size_t nextToSend = 0;

    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    AOptional<RpcError> error;
    _<AWebsocket> websocket;

    void finish(AOptional<RpcError> e = std::nullopt) {
        std::unique_lock lock(mutex);
        if (done) {
            return;
        }
        done = true;
        error = std::move(e);
        if (websocket) {
            websocket->close();
        }
        cv.notify_all();
    }

    void sendNext(AWebsocket& ws) {
        const auto& request = requests[nextToSend];
        AJson json = AJson::Object {};
        json["jsonrpc"] = "2.0";
        json["id"] = int(nextToSend + 1);
        json["method"] = request.method;
        if (!request.params.isEmpty() && !request.params.isNull()) {
            json["params"] = request.params;
        }
        ws.writeText(AJson::toString(json));
    }

    void onMessage(AWebsocket& ws, AByteBufferView payload) {
        AJson json;
        try {
            json = AJson::fromString(AString::fromUtf8(payload));
        } catch (const AException&) {
            return;   // not a JSON-RPC message; i.e., a pong
        }
        if (!json.isObject()) {
            return;
        }
        auto id = json.containsOpt("id");
        if (!id || !id->isInt() || id->asInt() != int(nextToSend + 1)) {
            return;   // notification or someone else's answer
        }
        if (auto e = json.containsOpt("error"); e && e->isObject()) {
            AJson data;
            if (auto d = e->containsOpt("data")) {
                data = *d;
            }
            finish(RpcError(
                e->containsOpt("code") ? (*e)["code"].asInt() : rpc_code::CLIENT_ERROR,
                e->containsOpt("message") ? (*e)["message"].asString() : AString("unknown error"), std::move(data)));
            return;
        }
        if (auto r = json.containsOpt("result")) {
            results << *r;
        } else {
            results << AJson(nullptr);
        }
        ++nextToSend;
        if (nextToSend == requests.size()) {
            finish();
        } else {
            sendNext(ws);
        }
    }
};

}   // namespace

AVector<AJson> RpcClient::callSequence(const AVector<RpcRequest>& requests, std::chrono::milliseconds timeout) const {
    auto session = std::make_shared<Session>();
    session->requests = requests;
    const auto urlString = url();

    // AWebsocket::run blocks and delivers its signals on the thread it runs on, so it gets a thread of its own.
    std::thread worker([session, urlString] {
        try {
            auto ws = _new<AWebsocket>(urlString);
            {
                std::unique_lock lock(session->mutex);
                if (session->done) {
                    return;
                }
                session->websocket = ws;
            }
            AObject::connect(ws->connected, ws, [session, &ws = *ws] { session->sendNext(ws); });
            AObject::connect(ws->received, ws, [session, &ws = *ws](AByteBufferView payload) {
                session->onMessage(ws, payload);
            });
            AObject::connect(ws->websocketClosed, ws, [session](const AString&) {
                session->finish(RpcError(rpc_code::CLIENT_ERROR, "the application closed the connection"));
            });
            ws->run();
            session->finish(RpcError(rpc_code::CLIENT_ERROR, "the connection was closed"));
        } catch (const AException& e) {
            session->finish(RpcError(
                rpc_code::CLIENT_ERROR, "can't talk to the application: " + e.getMessage(),
                AJson { { "hint", "check that the application is running and the key is valid (pair first)" } }));
        }
        std::unique_lock lock(session->mutex);
        session->websocket = nullptr;
    });

    {
        std::unique_lock lock(session->mutex);
        if (!session->cv.wait_for(lock, timeout, [&] { return session->done; })) {
            session->done = true;
            session->error = RpcError(
                rpc_code::CLIENT_ERROR, "timeout waiting for the answer to " + requests[session->nextToSend].method,
                AJson { { "hint", "the application is busy or hung; try again or raise the timeout" } });
            if (session->websocket) {
                session->websocket->close();
            }
        }
    }
    worker.join();

    if (session->error) {
        throw *session->error;
    }
    return std::move(session->results);
}

AJson RpcClient::call(AString method, AJson params, std::chrono::milliseconds timeout) const {
    return std::move(callSequence({ RpcRequest { std::move(method), std::move(params) } }, timeout).at(0));
}

}   // namespace aui_remote
