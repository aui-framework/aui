/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ARemoteAgent.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <random>

#include "AUI/Crypt/ATlsIdentity.h"
#include "AUI/Curl/AWebsocketServer.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/Json/AJson.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Platform/Entry.h"
#include "Agent/JsonRpc.h"
#include "Agent/Methods.h"
#include "Agent/Pairing.h"
#include "Agent/RefTable.h"

using namespace aui::remote;

namespace {
constexpr auto LOG_TAG = "ARemoteAgent";
constexpr int PROTOCOL_VERSION = 1;

std::mutex gMutex;
_<ARemoteAgent>& slot() {
    static _<ARemoteAgent> agent;
    return agent;
}

}   // namespace

struct ARemoteAgent::Impl {
    Options options;
    Pairing pairing;
    _<ATlsIdentity> identity;
    APath discoveryFile;
    RefTable refs;
    FunctionRegistry functions;
    JsonRpcDispatcher dispatcher;
    std::unique_ptr<AWebsocketServer> server;
};

ARemoteAgent::ARemoteAgent(Options options) : mImpl(std::make_unique<Impl>()) {
    auto& impl = *mImpl;
    impl.options = std::move(options);
    if (!impl.options.key.empty()) {
        impl.pairing.authorize(impl.options.key.toStdString());
    }

    auto& dispatcher = impl.dispatcher;
    dispatcher.registerMethod("session.hello", JsonRpcDispatcher::SyncHandler([this](const AJson&) {
        aui::impl::JsonArray methods;
        for (const auto& name : mImpl->dispatcher.methodNames()) {
            methods.push_back(AJson(name));
        }
        return AJson {
            { "protocol", PROTOCOL_VERSION },
            { "pid", int(AProcess::self()->getPid()) },
            { "methods", AJson(std::move(methods)) },
        };
    }));
    registerInspectionMethods(dispatcher, impl.refs);
    registerScreenshotMethods(dispatcher, impl.refs);
    registerInputMethods(dispatcher, impl.refs);
    registerWaitMethods(dispatcher, impl.refs);
    registerFunctionMethods(dispatcher, impl.functions);
    registerDiagnosticsMethods(dispatcher);

    // always encrypted: the key and everything on the screen must not be readable from the network
    impl.identity = ATlsIdentity::generateSelfSigned("aui-remote-agent");

    AWebsocketServer::Config config;
    config.tls = impl.identity;
    config.bindAddress = impl.options.bindAddress;
    config.port = impl.options.port;
    config.verifyHandshake = [](const AWebsocketServer::Request& request) {
        // browsers always send Origin; a web page must not be able to even try to pair
        return !request.headers.contains("origin");
    };
    config.onConnected = [this](_<AWebsocketServer::Connection> connection) {
        _weak<AWebsocketServer::Connection> weak = connection;
        // a client presents the key it got paired with
        std::string key;
        if (auto query = connection->request().queryParameter("key")) {
            key = query->toStdString();
        }
        auto authorized = std::make_shared<std::atomic<bool>>(mImpl->pairing.isAuthorized(key));
        auto respond = [weak](AString response) {
            // the response may be ready after the client left
            if (auto c = weak.lock()) {
                c->sendText(response);
            }
        };
        connection->onMessage = [this, authorized, respond](AByteBuffer message, bool) {
            const auto text = AString::fromUtf8(message);
            if (*authorized) {
                mImpl->dispatcher.handle(text, respond);
                return;
            }
            respond(AJson::toString(handleUnauthorized(text, *authorized)));
        };
    };
    impl.server = std::make_unique<AWebsocketServer>(std::move(config));
    impl.server->start();

    if (impl.options.writeDiscoveryFile) {
        try {
            const auto directory = APath::getDefaultPath(APath::HOME) / ".aui" / "remote";
            directory.makeDirs();
            impl.discoveryFile = directory / (AString::number(int(AProcess::self()->getPid())) + ".json");

            aui::impl::JsonArray argv;
            for (const auto& arg : aui::args()) {
                argv.push_back(AJson(arg));
            }
            AJson info {
                { "protocol", PROTOCOL_VERSION },
                { "pid", int(AProcess::self()->getPid()) },
                { "port", int(impl.server->port()) },
                { "scheme", "wss" },
                // a client pins the self-signed certificate by this fingerprint
                { "certSha256", impl.identity->fingerprintHex() },
                { "exe", AString(AProcess::self()->getPathToExecutable()) },
                { "argv", AJson(std::move(argv)) },
                { "cwd", AString(std::filesystem::current_path().string()) },
                { "startedAt", static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                           std::chrono::system_clock::now().time_since_epoch())
                                                           .count()) },
            };
            // keep the file private anyway
            {
                AFileOutputStream(impl.discoveryFile) << AJson::toString(info);
            }
            std::error_code ec;
            std::filesystem::permissions(impl.discoveryFile.toStdString(),
                                         std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                                         std::filesystem::perm_options::replace, ec);
        } catch (const AException& e) {
            ALogger::warn(LOG_TAG) << "Unable to write the discovery file: " << e;
            impl.discoveryFile = {};
        }
    }

    ALogger::info(LOG_TAG) << "Remote agent listens on wss://127.0.0.1:" << impl.server->port()
                           << (impl.discoveryFile.empty() ? AString {} : " (" + AString(impl.discoveryFile) + ")");
}

ARemoteAgent::~ARemoteAgent() {
    if (mImpl->server) {
        mImpl->server->stop();
    }
    if (!mImpl->discoveryFile.empty()) {
        try {
            mImpl->discoveryFile.removeFile();
        } catch (const AException&) {
        }
    }
}

_<ARemoteAgent> ARemoteAgent::start(Options options) {
    std::lock_guard lock(gMutex);
    auto& agent = slot();
    if (!agent) {
        agent = aui::ptr::manage_shared(new ARemoteAgent(std::move(options)));
    }
    return agent;
}

AJson ARemoteAgent::handleUnauthorized(const AString& text, std::atomic<bool>& authorized) {
    AJson id(nullptr);
    try {
        auto request = AJson::fromString(text);
        if (!request.isObject()) {
            return jsonRpcError(id, rpc_code::UNAUTHORIZED, "not paired", "call pair.begin, then pair.confirm");
        }
        if (auto i = request.containsOpt("id")) {
            id = *i;
        }
        const auto method = request.containsOpt("method") ? request["method"].asString() : AString {};
        const auto params = request.containsOpt("params") ? request["params"] : AJson(nullptr);
        if (method == "pair.begin") {
            return AJson { { "jsonrpc", "2.0" }, { "id", id }, { "result", mImpl->pairing.begin() } };
        }
        if (method == "pair.confirm") {
            const auto key = params["key"].asString().toStdString();
            auto result = mImpl->pairing.confirm(params["code"].asString(), key);
            authorized = true;
            return AJson { { "jsonrpc", "2.0" }, { "id", id }, { "result", std::move(result) } };
        }
        return jsonRpcError(id, rpc_code::UNAUTHORIZED, "not paired",
                            "call pair.begin, read the code in the application window and send it with pair.confirm");
    } catch (const RpcException& e) {
        return jsonRpcError(id, e.code(), e.getMessage(), e.hint());
    } catch (const AException& e) {
        return jsonRpcError(id, rpc_code::INVALID_PARAMS, e.getMessage(), "pair.confirm takes { code, key }");
    }
}

void ARemoteAgent::registerFunction(AString name, AString description,
                                    std::function<AFuture<AJson>(const AJson&)> handler) {
    mImpl->functions.add(std::move(name), std::move(description), std::move(handler));
}

_<ARemoteAgent> ARemoteAgent::start() { return start(Options {}); }

_<ARemoteAgent> ARemoteAgent::running() {
    std::lock_guard lock(gMutex);
    return slot();
}

void ARemoteAgent::startIfRequested() {
    const char* value = std::getenv("AUI_REMOTE");
    if (value == nullptr || std::string_view(value) != "1") {
        return;
    }
    try {
        Options options;
        if (const char* key = std::getenv("AUI_REMOTE_KEY")) {
            options.key = key;
        }
        start(std::move(options));
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to start the remote agent: " << e;
    }
}

std::uint16_t ARemoteAgent::port() const noexcept { return mImpl->server->port(); }

const APath& ARemoteAgent::discoveryFile() const noexcept { return mImpl->discoveryFile; }

AString ARemoteAgent::certificateFingerprint() const { return mImpl->identity->fingerprintHex(); }

Pairing& ARemoteAgent::pairing() noexcept { return mImpl->pairing; }

JsonRpcDispatcher& ARemoteAgent::dispatcher() noexcept { return mImpl->dispatcher; }
