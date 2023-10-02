#include "VPXDecoder.h"
#include "AUI/Logging/ALogger.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_decoder.h"
#include "libyuv/planar_functions.h"

namespace aui::video::impl {
    AImage convertToRGBA(vpx_image_t* image) {
        AByteBuffer buffer;
        buffer.resize(4 * image->r_w * image->r_h);

        switch (image->fmt) {
            case VPX_IMG_FMT_YV12:
                libyuv::I420ToABGR(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_V], image->stride[VPX_PLANE_V],
                                   image->planes[VPX_PLANE_U], image->stride[VPX_PLANE_U],
                                   reinterpret_cast<uint8_t*>(buffer.data()), 4 * int(image->r_w),
                                   int(image->r_w), int(image->r_h)
                );
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
                libyuv::I422ToABGR(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_U], image->stride[VPX_PLANE_U],
                                   image->planes[VPX_PLANE_V], image->stride[VPX_PLANE_V],
                                   reinterpret_cast<uint8_t*>(buffer.data()), 4 * int(image->r_w),
                                   int(image->r_w), int(image->r_h)
                );
                break;
            case VPX_IMG_FMT_I444:
                libyuv::I444ToABGR(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_U], image->stride[VPX_PLANE_U],
                                   image->planes[VPX_PLANE_V], image->stride[VPX_PLANE_V],
                                   reinterpret_cast<uint8_t*>(buffer.data()), 4 * int(image->r_w),
                                   int(image->r_w), int(image->r_h)
                );
                break;
//            case VPX_IMG_FMT_I440:
//                break;
            case VPX_IMG_FMT_NV12:
                libyuv::NV12ToABGR(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_PACKED], image->stride[VPX_PLANE_PACKED],
                                   reinterpret_cast<uint8_t*>(buffer.data()), 4 * int(image->r_w),
                                   int(image->r_w), int(image->r_h)
                );
                break;
//            case VPX_IMG_FMT_I42016:
//                break;
//            case VPX_IMG_FMT_I42216:
//                break;
//            case VPX_IMG_FMT_I44416:
//                break;
//            case VPX_IMG_FMT_I44016:
//                break;

            default:
                throw AException("unsupported pixel format");
        }

        return {std::move(buffer), {image->r_w, image->r_h}, APixelFormat::RGBA_BYTE};
    }
}

VPXDecoder::VPXDecoder(Codec codec) {
    auto codecFactory = [codec]() {
        switch (codec) {
            case Codec::VP8:
                return vpx_codec_vp8_dx;
            case Codec::VP9:
                return vpx_codec_vp9_dx;
            default:
                throw AException("VPX decoder: unknown codec");
        }
    }();
    if (auto code = vpx_codec_dec_init(mContext.ptr(), codecFactory(), nullptr, 0)) {
        throw AException("Failed to initialize VPX codec, error code: {}"_format(code));
    }
}

VPXDecoder::~VPXDecoder() {
    vpx_codec_destroy(mContext.ptr());
}

AVideoFrame VPXDecoder::decode(const AEncodedFrame& encodedFrame) {
    if (auto code = vpx_codec_decode(mContext.ptr(),
                                     reinterpret_cast<const uint8_t*>(encodedFrame.frameData.data()), encodedFrame.frameData.size(),
                                     nullptr, 0)) {
        throw AException("(VPX) failed to decode encoded frame, error code: {}"_format(code));
    }

    vpx_codec_iter_t iter = nullptr;

    if (auto image = vpx_codec_get_frame(mContext.ptr(), &iter)) {
        return AVideoFrame {
            .image = aui::video::impl::convertToRGBA(image),
            .timecode = encodedFrame.timecode
        };
    }

    throw AException("(VPX) failed to get decoded frame, error code");
}
