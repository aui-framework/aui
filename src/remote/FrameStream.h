#pragma once

#include <atomic>
#include <memory>

#include <glm/glm.hpp>

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Image/AImage.h>
#include <AUI/Json/AJson.h>
#include <AUI/Thread/AFuture.h>

#include "Discovery.h"
#include "RpcClient.h"
#include "ViewTree.h"

namespace aui_remote {

/**
 * @brief A frame pushed by the application, ready to be shown.
 */
struct FrameUpdate {
    int seq = 0;

    /**
     * @brief Logical size of the window of the application; the coordinates of the tree and the input are in it.
     */
    glm::ivec2 windowSize { 0, 0 };

    /**
     * @brief Decoded frame, downscaled by the subscription scale.
     */
    _<AImage> image;

    /**
     * @brief View tree as of this frame. Immutable; stays valid after the next frames arrive.
     */
    _<const ViewTree> tree;
};

/**
 * @brief Interactive session with an application: one persistent wss connection that receives the pushed frames and
 * carries the input.
 * @details
 * Unlike RpcClient, which connects per command, the stream holds the connection until stop(). The network IO and the
 * JPEG decoding are done on worker threads; the signals are emitted on the UI thread (the main thread of the
 * application). The object must be created with `_new`.
 *
 * The client acknowledges every applied frame (`frames.ack`); the agent builds the tree diff against the last
 * acknowledged one. A diff that does not match the held tree (desync) fails the stream.
 */
class FrameStream : public AObject {
public:
    FrameStream(AppInfo app, AString key);
    ~FrameStream() override;

    /**
     * @brief Connects and subscribes (`frames.subscribe`). Does nothing if already started. Call it on the UI thread:
     * the signals are delivered to the calling thread.
     * @param scale downscale of the frame, (0, 1]
     * @param quality JPEG quality, 1..100
     */
    void start(float scale = 1.f, aui::ranged_number<int, 0, 100> quality = 50);

    /**
     * @brief Closes the connection; the subscription dies with it. After that start() may be called again.
     */
    void stop();

    [[nodiscard]] bool isStarted() const noexcept;

    /**
     * @brief Fire-and-forget request on the connection (i.e., `input.*`). The response is only logged on error.
     */
    void call(AString method, AJson params = {});

    /**
     * @brief Request on the connection.
     * @return `result` of the response; fails with RpcError on an error response, a lost connection or stop()
     */
    AFuture<AJson> request(AString method, AJson params = {});

    /**
     * @return code of the last error (RpcError::code(), i.e., rpc_code::UNAUTHORIZED); 0 if there were none. Valid in
     * the slot of `failed`.
     */
    [[nodiscard]] int lastErrorCode() const noexcept { return mLastErrorCode; }

signals:
    /**
     * @brief Emitted for every frame, on the thread that called start() (the UI thread).
     */
    emits<FrameUpdate> frameReceived;

    /**
     * @brief The connection is lost or broken; the stream is stopped. The argument is the message (with the hint).
     */
    emits<AString> failed;

private:
    struct Session;
    AppInfo mApp;
    AString mKey;
    std::shared_ptr<Session> mSession;
    std::atomic_int mLastErrorCode = 0;

    void fail(const std::shared_ptr<Session>& session, int code, AString message);
};

}   // namespace aui_remote
