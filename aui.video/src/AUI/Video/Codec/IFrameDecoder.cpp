#include "IFrameDecoder.h"
#include "AUI/Video/Codec/vpx/VPXDecoder.h"
#include "vpx/vpx_decoder.h"

_<IFrameDecoder> IFrameDecoder::fromCodec(aui::video::Codec codec) {
    switch (codec) {
        case aui::video::Codec::VP8:
            return _new<VPXDecoder>(VPX::Codec::VP8);

        case aui::video::Codec::VP9:
            return _new<VPXDecoder>(VPX::Codec::VP9);
    }

    return nullptr;
}