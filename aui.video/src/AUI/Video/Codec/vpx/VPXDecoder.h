#pragma once

#include "AUI/Video/IFrameDecoder.h"
#include "AUI/Util/APimpl.h"

typedef struct vpx_codec_ctx vpx_codec_ctx_t;

class VPXDecoder : public IFrameDecoder {
public:
    enum class Codec {
        VP8,
        VP9
    };

    explicit VPXDecoder(Codec codec);

    ~VPXDecoder();

    AVideoFrame decode(const AEncodedFrame& encodedFrame) override;

private:
    aui::fast_pimpl<vpx_codec_ctx_t, 5 * sizeof(void*) + 2 * sizeof(long), std::max(sizeof(void*), sizeof(long))> mContext;
};
