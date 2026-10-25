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

#include "AUI/Curl/AWebsocketServer.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/Json/AJson.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Platform/Entry.h"
#include "Agent/JsonRpc.h"
#include "Agent/Methods.h"
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

AString randomToken() {
    // 128 bits from the operating system's entropy source
    std::random_device device;
    static constexpr char HEX[] = "0123456789abcdef";
    std::string token;
    for (int i = 0; i < 16; ++i) {
        const auto byte = static_cast<std::uint8_t>(device());
        token.push_back(HEX[byte >> 4]);
        token.push_back(HEX[byte & 0xf]);
    }
    return AString(token);
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    unsigned difference = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        difference |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return difference == 0;
}
}   // namespace

struct ARemoteAgent::Impl {
    Options options;
    AString token;
    APath discoveryFile;
    RefTable refs;
    FunctionRegistry functions;
    JsonRpcDispatcher dispatcher;
    std::unique_ptr<AWebsocketServer> server;
};

ARemoteAgent::ARemoteAgent(Options options) : mImpl(std::make_unique<Impl>()) {
    auto& impl = *mImpl;
    impl.options = std::move(options);
    impl.token = impl.options.token.empty() ? randomToken() : impl.options.token;

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

    AWebsocketServer::Config config;
    config.bindAddress = impl.options.bindAddress;
    config.port = impl.options.port;
    config.verifyHandshake = [token = impl.token.toStdString()](const AWebsocketServer::Request& request) {
        // browsers always send Origin; a web page must not be able to drive the application
        if (request.headers.contains("origin")) {
            return false;
        }
        std::string presented;
        if (auto query = request.queryParameter("token")) {
            presented = query->toStdString();
        } else if (auto it = request.headers.find("authorization"); it != request.headers.end()) {
            constexpr std::string_view PREFIX = "bearer ";
            auto value = it->second.toStdString();
            if (value.size() > PREFIX.size() && AString(value.substr(0, PREFIX.size())).lowercase() == AString(PREFIX)) {
                presented = value.substr(PREFIX.size());
            }
        }
        return constantTimeEquals(presented, token);
    };
    config.onConnected = [this](_<AWebsocketServer::Connection> connection) {
        _weak<AWebsocketServer::Connection> weak = connection;
        connection->onMessage = [this, weak](AByteBuffer message, bool) {
            mImpl->dispatcher.handle(AString::fromUtf8(message), [weak](AString response) {
                // the response may be ready after the client left
                if (auto c = weak.lock()) {
                    c->sendText(response);
                }
            });
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
                { "token", impl.token },
                { "exe", AString(AProcess::self()->getPathToExecutable()) },
                { "argv", AJson(std::move(argv)) },
                { "cwd", AString(std::filesystem::current_path().string()) },
                { "startedAt", static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                           std::chrono::system_clock::now().time_since_epoch())
                                                           .count()) },
            };
            // the token is a secret: the file must be readable by its owner only
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

    ALogger::info(LOG_TAG) << "Remote agent listens on 127.0.0.1:" << impl.server->port()
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
        start();
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to start the remote agent: " << e;
    }
}

std::uint16_t ARemoteAgent::port() const noexcept { return mImpl->server->port(); }

const AString& ARemoteAgent::token() const noexcept { return mImpl->token; }

const APath& ARemoteAgent::discoveryFile() const noexcept { return mImpl->discoveryFile; }

JsonRpcDispatcher& ARemoteAgent::dispatcher() noexcept { return mImpl->dispatcher; }
