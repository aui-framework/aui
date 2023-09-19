#include "VPXDecoder.h"
#include "AUI/Logging/ALogger.h"
#include "vpx/vpx_decoder.h"
#include "tools_common.h"
#include "libyuv/convert_from.h"

//we must define usage_exit due to include tools_common.h
void usage_exit(void) {
    exit(-1);
}

namespace aui::video::impl {
    AImage convertToRGBA(vpx_image_t* image) {
        AByteBuffer buffer;
        buffer.resize(4 * image->r_w * image->r_h);

        switch (image->fmt) {
            case VPX_IMG_FMT_YV12:
                break;
            case VPX_IMG_FMT_I420:
                libyuv::I420ToABGR(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_U], image->stride[VPX_PLANE_U],
                                   image->planes[VPX_PLANE_V], image->stride[VPX_PLANE_V],
                                   reinterpret_cast<uint8_t*>(buffer.data()), 4 * int(image->r_w),
                                   int(image->r_w), int(image->r_h)
                );
                break;
            case VPX_IMG_FMT_I422:
                break;
            case VPX_IMG_FMT_I444:
                break;
            case VPX_IMG_FMT_I440:
                break;
            case VPX_IMG_FMT_NV12:
                break;
            case VPX_IMG_FMT_I42016:
                break;
            case VPX_IMG_FMT_I42216:
                break;
            case VPX_IMG_FMT_I44416:
                break;
            case VPX_IMG_FMT_I44016:
                break;

            default:
                throw AException("unknown pixel format");
        }

        return {std::move(buffer), {image->r_w, image->r_h}, APixelFormat::RGBA_BYTE};
    }
}

VPXDecoder::VPXDecoder(Codec codec) {
    auto decoder = get_vpx_decoder_by_name(codec == Codec::VP8 ? "vp8" : "vp9");
    if (auto code = vpx_codec_dec_init(mContext.ptr(), decoder->codec_interface(), nullptr, 0)) {
        throw AException("Failed to initialize VPX codec, error code: {}"_format(code));
    }
}

VPXDecoder::~VPXDecoder() {
    vpx_codec_destroy(mContext.ptr());
}

AFrame VPXDecoder::decode(const ACodedFrame& codedFrame) {
    if (auto code = vpx_codec_decode(mContext.ptr(),
                                 reinterpret_cast<const uint8_t*>(codedFrame.frameData.data()), codedFrame.frameData.size(),
                                nullptr, 0)) {
        throw AException("(VPX) failed to decode coded frame, error code: {}"_format(code));
    }

    vpx_codec_iter_t iter = nullptr;

    if (auto image = vpx_codec_get_frame(mContext.ptr(), &iter)) {
        return AFrame {
            .image = aui::video::impl::convertToRGBA(image),
            .timecode = codedFrame.timecode
        };
    }

    throw AException("(VPX) failed to get decoded frame, error code");
}
