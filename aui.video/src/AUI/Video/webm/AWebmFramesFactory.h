#pragma once

#include "AWebmFrameBuffer.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Util/APimpl.h"

namespace webm {
    class WebmParser;
}
class MyWebmReader;
class MyWebmCallback;
typedef struct vpx_codec_ctx vpx_codec_ctx_t;
typedef struct VpxInterface VpxInterface;

class AWebmFramesFactory : public IImageFactory {
public:
    explicit AWebmFramesFactory(_<IInputStream> stream);

    ~AWebmFramesFactory();

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

private:
    std::chrono::time_point<std::chrono::system_clock> mPlaybackStarted;
    AWebmFrameBuffer mFrameBuffer;

    _<MyWebmReader> mReader;
    _<MyWebmCallback> mCallback;
    _<webm::WebmParser> mParser;

    _<vpx_codec_ctx> mContext;
    const VpxInterface* mDecoder = nullptr;

    void onFrameLoaded(AByteBuffer buffer, int16_t timecode);

    void onVideoTrackInfoParsed(std::string_view codecName);

    friend class MyWebmCallback;
};
