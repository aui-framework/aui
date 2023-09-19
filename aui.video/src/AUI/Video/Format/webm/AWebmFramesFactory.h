#pragma once

#include "AUI/Video/AFrameBuffer.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Util/APimpl.h"
#include "AUI/Thread/AFuture.h"

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
    AFuture<> mProcessingFuture;
    std::chrono::time_point<std::chrono::system_clock> mPlaybackStarted;
    AFrameBuffer mFrameBuffer;
    aui::fast_pimpl<vpx_codec_ctx_t, 5 * sizeof(void*) + 2 * sizeof(long), std::max(sizeof(void*), sizeof(long))> mContext;

    void onFrameLoaded(AByteBufferView buffer, int16_t timecode);

    void onVideoTrackInfoParsed(std::string_view codecName);

    friend class MyWebmCallback;
};
