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

#include "Methods.h"

#include <deque>

#include "AUI/Common/AObject.h"
#include "AUI/Image/AImage.h"
#include "AUI/Image/jpg/JpgImageLoader.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Reflect/AReflect.h"
#include "AUI/View/AViewContainer.h"
#include "AUI/Thread/AThreadPool.h"
#include "RemoteOverlay.h"
#include "TreeDiff.h"
#include "UiThread.h"
#include "ViewJson.h"

namespace aui::remote {

namespace {

constexpr auto LOG_TAG = "FrameMethods";
constexpr std::size_t MAX_UNACKED_TREES = 64;

struct Params {
    float scale = 0.5f;
    int quality = 70;
};

Params parseParams(const AJson& json) {
    Params params;
    if (auto scale = json.containsOpt("scale"); scale && !scale->isNull()) {
        params.scale = float(scale->asNumber());
        if (!(params.scale > 0.f && params.scale <= 1.f)) {
            throw RpcException(rpc_code::INVALID_PARAMS, "scale must be in (0, 1]");
        }
    }
    if (auto quality = json.containsOpt("quality"); quality && !quality->isNull()) {
        params.quality = quality->asInt();
        if (params.quality < 1 || params.quality > 100) {
            throw RpcException(rpc_code::INVALID_PARAMS, "quality must be in 1..100");
        }
    }
    return params;
}

/// Everything about the view that goes into the tree. Refs are stable as long as the RefTable remembers the view.
void collectTree(RefTable& refs, const _<AView>& view, tree_diff::Tree& tree, const AString& ref) {
    tree_diff::Node node;
    node.type = view_json::typeName(view);
    if (auto text = view_json::textOf(view); !text.empty()) {
        node.text = std::move(text);
    }
    if (const auto& names = view->getAssNames(); !names.empty()) {
        AString joined;
        for (const auto& name : names) {
            joined += (joined.empty() ? "" : " ") + name;
        }
        node.name = std::move(joined);
    }
    const auto position = view->getPosition();
    const auto size = view->getSize();
    const auto absolute = view->getPositionInWindow();
    node.x = position.x;
    node.y = position.y;
    node.w = size.x;
    node.h = size.y;
    node.absX = absolute.x;
    node.absY = absolute.y;
    node.visible = view_json::isVisible(view);
    if (auto container = _cast<AViewContainer>(view); container && container->getLayout()) {
        node.layout = AReflect::name(container->getLayout().get());
    }
    const auto fields = [](const ABoxFields& f) { return std::array { f.left, f.top, f.right, f.bottom }; };
    node.padding = fields(view->getPadding());
    node.margin = fields(view->getMargin());

    const auto& children = view_json::childrenOf(view);
    AVector<AString> childRefs;
    childRefs.reserve(children.size());
    for (const auto& child : children) {
        childRefs.push_back(refs.add(child));
    }
    node.children = childRefs;
    tree.nodes[ref] = std::move(node);
    for (std::size_t i = 0; i < children.size(); ++i) {
        collectTree(refs, children[i], tree, childRefs[i]);
    }
}

tree_diff::Tree buildTree(RefTable& refs, const _<AWindow>& window) {
    tree_diff::Tree tree;
    tree.root = refs.add(window);
    collectTree(refs, window, tree, tree.root);
    return tree;
}

/// A frame ready to leave: the heavy part (downscale, encoding, serialization) is done off the UI thread.
AString encodeFrame(AImage image, Params params, std::int64_t seq, glm::ivec2 windowSize, float scalingFactor,
                    const tree_diff::Diff& treeDiff) {
    if (params.scale < 1.f) {
        const glm::uvec2 target = glm::max(glm::uvec2(glm::vec2(image.size()) * params.scale + 0.5f), glm::uvec2(1));
        image = image.resizedLinearDownscale(target);
    }
    AByteBuffer jpeg;
    JpgImageLoader::save(jpeg, image, params.quality);
    AJson frame {
        { "seq", seq },
        { "width", int(image.width()) },
        { "height", int(image.height()) },
        { "windowWidth", windowSize.x },
        { "windowHeight", windowSize.y },
        { "scalingFactor", scalingFactor },
        { "jpeg", jpeg.toBase64String() },
    };
    frame["tree"] = tree_diff::toJson(treeDiff);
    return AJson::toString(AJson {
        { "jsonrpc", "2.0" },
        { "method", "frames.frame" },
        { "params", std::move(frame) },
    });
}

/**
 * @brief The stream of frames of one window to one connection. UI thread only (except the sending part).
 * @details
 * A frame is captured at the end of a render, right before the remote control overlay draws itself (see
 * RemoteOverlay::addBeforeDraw), so it is clean and costs no render of its own (which would be presented and make the
 * overlay flicker). While a frame is being sent, new renders only leave a mark; when the send completes, the mark
 * asks for a fresh render.
 */
class Subscription : public AObject {
public:
    Subscription(RefTable& refs, _weak<RpcConnection> connection, _<AWindow> window, Params params, std::int64_t seq)
      : mRefs(refs), mConnection(std::move(connection)), mWindow(window), mParams(params), mSeq(seq) {
        RemoteOverlay::enable(*window);
        RemoteOverlay::addBeforeDraw(*window, this, [this] { onRendered(); });
    }

    ~Subscription() override {
        if (auto window = mWindow.lock()) {
            RemoteOverlay::removeBeforeDraw(*window, this);
        }
    }

    [[nodiscard]] const _weak<RpcConnection>& connection() const noexcept { return mConnection; }
    [[nodiscard]] std::int64_t seq() const noexcept { return mSeq; }

    /// Makes sure a frame comes soon.
    void schedule() {
        if (auto window = mWindow.lock()) {
            window->flagRedraw();
        }
    }

    void ack(std::int64_t seq) {
        auto it = mUnacked.find(seq);
        if (it == mUnacked.end()) {
            // never sent, already acknowledged or forgotten
            ALogger::warn(LOG_TAG) << "ack of frame " << seq << " which is not awaiting one; ignored";
            return;
        }
        ALogger::debug(LOG_TAG) << "frame " << seq << " acknowledged";
        mAcked = std::make_pair(seq, std::move(it->second));
        // the frames before the acked one can never become a base
        mUnacked.erase(mUnacked.begin(), std::next(it));
    }

private:
    _weak<Subscription> weakPtr() { return aui::ptr::shared_from_this(this); }
    RefTable& mRefs;
    _weak<RpcConnection> mConnection;
    _weak<AWindow> mWindow;
    Params mParams;
    std::int64_t mSeq;
    bool mInFlight = false;
    bool mDirty = false;
    AOptional<std::pair<std::int64_t, tree_diff::Tree>> mAcked;
    std::map<std::int64_t, tree_diff::Tree> mUnacked;

    void onRendered() {
        if (mInFlight) {
            mDirty = true;
            return;
        }
        produce();
    }
    void produce();
    void onSent() {
        mInFlight = false;
        if (std::exchange(mDirty, false)) {
            schedule();
        }
    }
};

void Subscription::produce() {
    auto window = mWindow.lock();
    auto connection = mConnection.lock();
    if (!window || !connection || mInFlight) {
        return;
    }
    mDirty = false;

    AImage image;
    try {
        image = window->getRenderingContext()->makeScreenshot();
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "unable to capture the window: " << e.getMessage();
        return;
    }
    if (image.width() == 0 || image.height() == 0) {
        ALogger::warn(LOG_TAG) << "the rendering context returned an empty screenshot; no frame";
        return;
    }

    auto tree = buildTree(mRefs, window);
    const auto seq = ++mSeq;
    auto treeDiff = mAcked ? tree_diff::diff(mAcked->second, mAcked->first, tree) : tree_diff::diff(tree);
    ALogger::debug(LOG_TAG) << "frame " << seq << ": " << tree.nodes.size() << " views, "
                            << (mAcked ? "diff against frame " + AString::number(mAcked->first) : AString("full tree"));
    mUnacked[seq] = std::move(tree);
    while (mUnacked.size() > MAX_UNACKED_TREES) {
        ALogger::warn(LOG_TAG) << "the client doesn't acknowledge frames (" << mUnacked.size()
                               << " awaiting); forgetting the frame " << mUnacked.begin()->first;
        mUnacked.erase(mUnacked.begin());
    }

    mInFlight = true;
    AThreadPool::global().run([weak = weakPtr(), connection = std::move(connection), image = std::move(image),
                               params = mParams, seq, windowSize = window->getSize(),
                               scalingFactor = window->getScalingParams().scalingFactor,
                               treeDiff = std::move(treeDiff)]() mutable {
        try {
            connection->sendText(encodeFrame(std::move(image), params, seq, windowSize, scalingFactor, treeDiff));
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "unable to send frame " << seq << ": " << e.getMessage();
        }
        connection.reset();
        AThread::main()->enqueue([weak] {
            if (auto self = weak.lock()) {
                self->onSent();
            }
        });
    });
}

/// Subscriptions by connection. UI thread only.
struct Registry {
    AMap<RpcConnection*, _<Subscription>> subscriptions;

    _<Subscription> find(const _<RpcConnection>& connection) {
        auto it = subscriptions.find(connection.get());
        if (it == subscriptions.end()) {
            return nullptr;
        }
        if (it->second->connection().lock() != connection) {
            // an address of a connection that is long gone
            subscriptions.erase(it);
            return nullptr;
        }
        return it->second;
    }
};

_<RpcConnection> requireConnection(const _<RpcConnection>& connection) {
    if (!connection) {
        throw RpcException(rpc_code::NOT_SUPPORTED, "frames need a connection to push to");
    }
    return connection;
}

AJson emptyResult() { return AJson::fromString("{}"); }

}   // namespace

void registerFrameMethods(JsonRpcDispatcher& dispatcher, RefTable& refs) {
    auto registry = std::make_shared<Registry>();

    // params: windowId?, scale? (0..1, default 0.5), quality? (1..100, default 70)
    // result: { subscribed: true }
    dispatcher.registerConnectionMethod(
        "frames.subscribe",
        JsonRpcDispatcher::ConnectionHandler([&refs, registry](const AJson& params, const _<RpcConnection>& connection) {
            requireConnection(connection);
            return onUiThread([&refs, registry, params = params, connection] {
                auto window = windowFromParams(params, refs);
                const auto parsed = parseParams(params);
                // the new subscription replaces the old one and starts with a full tree; the numbering goes on
                std::int64_t seq = 0;
                if (auto previous = registry->find(connection)) {
                    seq = previous->seq();
                }
                auto subscription = _new<Subscription>(refs, connection, window, parsed, seq);
                registry->subscriptions[connection.get()] = subscription;
                connection->addCloseListener([registry, weak = _weak<Subscription>(subscription)] {
                    AThread::main()->enqueue([registry, weak] {
                        auto self = weak.lock();
                        if (self) {
                            ALogger::info(LOG_TAG) << "the client has left; subscription dropped";
                        }
                        for (auto it = registry->subscriptions.begin(); self && it != registry->subscriptions.end();
                             ++it) {
                            if (it->second == self) {
                                registry->subscriptions.erase(it);
                                break;
                            }
                        }
                    });
                });
                ALogger::info(LOG_TAG) << "subscribed: scale " << parsed.scale << ", quality " << parsed.quality
                                       << ", numbering goes on from " << seq;
                subscription->schedule();
                return AJson { { "subscribed", true } };
            });
        }));

    dispatcher.registerConnectionMethod(
        "frames.unsubscribe",
        JsonRpcDispatcher::ConnectionHandler([registry](const AJson&, const _<RpcConnection>& connection) {
            requireConnection(connection);
            return onUiThread([registry, connection] {
                if (registry->find(connection)) {
                    ALogger::info(LOG_TAG) << "unsubscribed";
                    registry->subscriptions.erase(connection.get());
                }
                return emptyResult();
            });
        }));

    // params: seq
    dispatcher.registerConnectionMethod(
        "frames.ack",
        JsonRpcDispatcher::ConnectionHandler([registry](const AJson& params, const _<RpcConnection>& connection) {
            requireConnection(connection);
            const auto seq = params["seq"].asLongInt();
            return onUiThread([registry, connection, seq] {
                if (auto subscription = registry->find(connection)) {
                    subscription->ack(seq);
                }
                return emptyResult();
            });
        }));
}

}   // namespace aui::remote
