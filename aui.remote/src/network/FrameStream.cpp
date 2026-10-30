#include "FrameStream.h"

#include <AUI/Crypt/ATlsSession.h>
#include <condition_variable>
#include <deque>
#include <map>
#include <thread>
#include <unordered_map>

#if !AUI_PLATFORM_WIN
#include <signal.h>
#endif

#include <AUI/Logging/ALogger.h>
#include <AUI/Thread/AThread.h>


using namespace std::chrono_literals;

namespace aui_remote {

namespace {

constexpr auto POLL_INTERVAL = 100ms;
constexpr auto CONNECT_TIMEOUT = 15s;

struct Failure {
    int code = rpc_code::CLIENT_ERROR;
    AString message;
};

Failure describe(const std::exception_ptr& e) {
    try {
        std::rethrow_exception(e);
    } catch (const RpcError& rpc) {
        Failure f { rpc.code(), AString(rpc.getMessage()) };
        if (rpc.data().isObject()) {
            if (auto hint = rpc.data().containsOpt("hint"); hint && hint->isString()) {
                f.message += ". " + hint->asString();
            }
        }
        return f;
    } catch (const ATlsException& tls) {
        return { rpc_code::CLIENT_ERROR, "secure connection failed: " + tls.getMessage() };
    } catch (const AException& ex) {
        return { rpc_code::CLIENT_ERROR, "can't talk to the application: " + ex.getMessage() };
    } catch (const std::exception& ex) {
        return { rpc_code::CLIENT_ERROR, AString("can't talk to the application: ") + ex.what() };
    }
}

AJson makeRequest(int id, const AString& method, const AJson& params) {
    AJson json = AJson::Object {};
    json["jsonrpc"] = "2.0";
    if (id != 0) {
        json["id"] = id;
    }
    json["method"] = method;
    if (!params.isEmpty() && !params.isNull()) {
        json["params"] = params;
    }
    return json;
}

}   // namespace

struct FrameStream::Session {
    struct Pending {
        /// empty for a request nobody waits for
        AOptional<AFuture<AJson>::Resolver> resolver;
        AString method;
        bool fireAndForget;
    };

    std::atomic_bool stopped = false;
    std::atomic_bool reported = false;
    std::mutex mutex;
    std::condition_variable cv;
    std::deque<AString> outbox;
    _<AWebsocket> connection;
    std::unordered_map<int, Pending> pending;
    int nextId = 1;

    /// @return id of the request, 0 for none (the session is stopped)
    int enqueue(const AString& method, const AJson& params, Pending pendingEntry, bool wantsId = true) {
        std::unique_lock lock(mutex);
        if (stopped) {
            return 0;
        }
        int id = wantsId ? nextId++ : 0;
        if (wantsId) {
            pending.emplace(id, std::move(pendingEntry));
        }
        outbox.push_back(AJson::toString(makeRequest(id, method, params)));
        cv.notify_all();
        return id;
    }

    void stop() {
        std::unordered_map<int, Pending> toFail;
        {
            std::unique_lock lock(mutex);
            stopped = true;
            toFail = std::move(pending);
            pending.clear();
            outbox.clear();
        }
        cv.notify_all();
        for (auto& [id, p] : toFail) {
            try {
                throw RpcError(rpc_code::CLIENT_ERROR, "the stream is stopped");
            } catch (...) {
                if (p.resolver) {
                    p.resolver->supplyException();
                }
            }
        }
    }
};

FrameStream::FrameStream(AppInfo app, AString key) : mApp(std::move(app)), mKey(std::move(key)) {}

FrameStream::~FrameStream() {
    if (mSession) {
        mSession->stop();
    }
}

bool FrameStream::isStarted() const noexcept { return mSession != nullptr; }

void FrameStream::fail(const std::shared_ptr<Session>& session, int code, AString message) {
    // UI thread
    if (mSession != session) {
        return;   // stale
    }
    mSession = nullptr;
    session->stop();
    mLastErrorCode = code;
    ALogger::err("FrameStream") << "stream failed (code " << code << "): " << message;
    emit failed(std::move(message));
}

void FrameStream::stop() {
    if (auto session = std::move(mSession)) {
        ALogger::info("FrameStream") << "stopped by the client";
        mSession = nullptr;
        session->stop();
    }
}

void FrameStream::start(float scale, aui::ranged_number<int, 0, 100> quality) {
    if (mSession) {
        return;
    }
    mLastErrorCode = 0;
#if !AUI_PLATFORM_WIN
    ::signal(SIGPIPE, SIG_IGN);
#endif
    auto session = std::make_shared<Session>();
    mSession = session;
    std::weak_ptr<AObject> weakSelf = weak_from_this();
    _<AAbstractThread> ui = AThread::current();   // the thread that calls start() is the one the signals go to

    // everything the threads report goes through here: posted to the UI thread
    auto postFailure = [session, weakSelf, ui](Failure failure) {
        if (session->reported.exchange(true)) {
            return;
        }
        session->stop();
        *ui * [session, weakSelf, failure = std::move(failure)] {
            auto self = std::static_pointer_cast<FrameStream>(weakSelf.lock());
            if (self) {
                self->fail(session, failure.code, failure.message);
            }
        };
    };

    // reader: connects, subscribes, applies and decodes the frames
    std::thread([session, weakSelf, postFailure, ui, app = mApp, key = mKey, scale, quality] {
        try {
            AString target = "/";
            if (!key.empty()) {
                target += "?key=" + key;
            }
            auto connection = connectToAgent(app.port, app.certSha256, target, CONNECT_TIMEOUT);
            {
                std::unique_lock lock(session->mutex);
                if (session->stopped) {
                    return;
                }
                session->connection = connection;
            }
            session->enqueue(
                "frames.subscribe", AJson { { "scale", double(scale) }, { "quality", quality } },
                Session::Pending { {}, "frames.subscribe", false });

            ALogger::info("FrameStream") << "connected to " << app.displayName() << ", port " << app.port
                                         << "; subscribing (scale " << scale << ", quality " << quality << ")";

            // The agent builds the diff against the last frame it has seen acknowledged, which may be older than the
            // newest frame applied here (the ack is still on its way). Hence the history.
            constexpr std::size_t MAX_HISTORY = 64;
            std::map<int, ViewTree> history;
            ViewTree tree;

            // The frame that has reached the UI. A frame equal to it is not passed on: showing it would make the
            // viewport render again for nothing. When aui-remote looks at its own window, every render makes a frame,
            // and the nested images of the viewport stop changing after a few, so this is what ends the loop.
            AString shownJpeg;
            glm::ivec2 shownWindowSize { 0, 0 };
            double shownScalingFactor = 0;
            ViewTree shownTree;
            while (!session->stopped) {
                auto text = connection->read(POLL_INTERVAL);
                if (!text) {
                    continue;
                }
                AJson message;
                try {
                    message = AJson::fromString(AString::fromUtf8(*text));
                } catch (const AJsonException&) {
                    continue;
                }
                if (!message.isObject()) {
                    continue;
                }
                if (auto id = message.containsOpt("id"); id && id->isInt()) {
                    Session::Pending entry;
                    {
                        std::unique_lock lock(session->mutex);
                        auto it = session->pending.find(id->asInt());
                        if (it == session->pending.end()) {
                            continue;
                        }
                        entry = std::move(it->second);
                        session->pending.erase(it);
                    }
                    if (auto e = message.containsOpt("error"); e && e->isObject()) {
                        AJson data;
                        if (auto d = e->containsOpt("data")) {
                            data = *d;
                        }
                        RpcError error(
                            e->containsOpt("code") ? (*e)["code"].asInt() : rpc_code::CLIENT_ERROR,
                            e->containsOpt("message") ? (*e)["message"].asString() : AString("unknown error"),
                            std::move(data));
                        if (entry.method == "frames.subscribe") {
                            postFailure(describe(std::make_exception_ptr(error)));
                            return;
                        }
                        if (entry.fireAndForget) {
                            ALogger::warn("FrameStream") << entry.method << " failed: " << error.getMessage();
                        } else {
                            entry.resolver->supplyException(std::make_exception_ptr(error));
                        }
                    } else if (!entry.fireAndForget && entry.method != "frames.subscribe") {
                        entry.resolver->supplyValue(message.containsOpt("result") ? message["result"] : AJson(nullptr));
                    }
                    continue;
                }
                auto method = message.containsOpt("method");
                if (!method || !method->isString() || method->asString() != "frames.frame") {
                    continue;
                }
                const auto& params = message["params"];
                const int seq = params["seq"].asInt();
                const auto& treeDiff = params["tree"];
                const auto base = treeDiff.containsOpt("base");
                const bool hasBase = base && base->isInt();
                if (hasBase) {
                    auto it = history.find(base->asInt());
                    if (it == history.end()) {
                        ALogger::err("FrameStream")
                            << "frame " << seq << " is built against frame " << base->asInt()
                            << ", which is not in the history (" << history.size() << " trees held)";
                        throw AException("view tree desync: frame {} is built against frame {}, which is gone"_format(
                            seq, base->asInt()));
                    }
                    tree = it->second;
                } else {
                    tree = ViewTree();
                }
                try {
                    tree.apply(treeDiff, seq);
                } catch (const AException& e) {
                    ALogger::err("FrameStream") << "unable to apply the tree of frame " << seq << ": " << e.getMessage();
                    throw;
                }
                history[seq] = tree;
                while (history.size() > MAX_HISTORY) {
                    history.erase(history.begin());
                }
                ALogger::debug("FrameStream") << "frame " << seq << " applied, "
                                              << (hasBase ? "base " + AString::number(base->asInt()) : AString("full"))
                                              << ", " << tree.size() << " views";
                session->enqueue("frames.ack", AJson { { "seq", seq } }, {}, false);

                const glm::ivec2 windowSize { params["windowWidth"].asInt(), params["windowHeight"].asInt() };
                double scalingFactor = 1.0;
                if (auto v = params.containsOpt("scalingFactor"); v && v->isNumber()) {
                    scalingFactor = v->asNumber();
                }
                const auto& jpeg = params["jpeg"].asString();
                if (jpeg == shownJpeg && windowSize == shownWindowSize && scalingFactor == shownScalingFactor &&
                    tree.sameContent(shownTree)) {
                    continue;
                }
                shownJpeg = jpeg;
                shownWindowSize = windowSize;
                shownScalingFactor = scalingFactor;
                shownTree = tree;

                FrameUpdate update;
                update.seq = seq;
                update.windowSize = windowSize;
                update.scalingFactor = float(scalingFactor);
                update.image = AImage::fromBuffer(AByteBuffer::fromBase64String(jpeg));
                if (!update.image) {
                    throw AException("can't decode the frame " + AString::number(seq));
                }
                update.tree = std::make_shared<const ViewTree>(tree);
                *ui * [session, weakSelf, update = std::move(update)]() mutable {
                    if (auto self = std::static_pointer_cast<FrameStream>(weakSelf.lock());
                        self && self->mSession == session) {
                        (*self) ^ self->frameReceived(update);
                    }
                };
            }
        } catch (...) {
            if (!session->stopped) {
                postFailure(describe(std::current_exception()));
            }
        }
    }).detach();

    // writer: sends the queue
    std::thread([session, postFailure] {
        try {
            for (;;) {
                AString text;
                _<AWebsocket> connection;
                {
                    std::unique_lock lock(session->mutex);
                    session->cv.wait(lock, [&] {
                        return session->stopped || (session->connection && !session->outbox.empty());
                    });
                    if (session->stopped) {
                        return;
                    }
                    connection = session->connection;
                    text = std::move(session->outbox.front());
                    session->outbox.pop_front();
                }
                connection->writeText(text);
            }
        } catch (...) {
            if (!session->stopped) {
                postFailure(describe(std::current_exception()));
            }
        }
    }).detach();
}

void FrameStream::call(AString method, AJson params) {
    if (!mSession) {
        ALogger::warn("FrameStream") << "call " << method << " on a stopped stream";
        return;
    }
    mSession->enqueue(method, params, Session::Pending { {}, method, true });
}

AFuture<AJson> FrameStream::request(AString method, AJson params) {
    // an exception, also from parsing the reply, destroys the resolver, so the request fails instead of hanging
    return AFuture<AJson>::create([&](AFuture<AJson>::Resolver resolver) {
        if (!mSession || !mSession->enqueue(method, params, Session::Pending { std::move(resolver), method, false })) {
            throw RpcError(rpc_code::CLIENT_ERROR, "the stream is not started");
        }
    });
}

}   // namespace aui_remote
