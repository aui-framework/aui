#include "Pairing.h"

#include "RpcClient.h"

namespace aui_remote::pairing {

Result confirm(const AppInfo& app, const AString& code, const KeyStore& store) {
    const auto key = KeyStore::generateKey();
    const auto answer = RpcClient::forApp(app, "").call("pair.confirm", AJson { { "code", code }, { "key", key } });
    if (!(answer.isObject() && answer.containsOpt("paired") && answer["paired"].asBool())) {
        throw RpcError(rpc_code::CLIENT_ERROR, "the application did not confirm the pairing",
                       AJson { { "hint", "start again" } });
    }
    // an agent that does not say anything is assumed to trust the key until it exits
    AString trust = "session";
    if (auto t = answer.containsOpt("trust"); t && t->isString()) {
        trust = t->asString();
    }
    Result result;
    if (trust == "persistent") {
        store.store(app, key);
        result.remembered = true;
    } else if (trust == "connection") {
        result.connectionOnly = true;
    } else {
        store.storeSession(app, key);
    }
    return result;
}

}   // namespace aui_remote::pairing
