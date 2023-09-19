#include "AWebmFramesFactory.h"
#include "AUI/Common/AQueue.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Logging/ALogger.h"

#include "webm/webm_parser.h"
#include "vpx/vpx_decoder.h"
#include "tools_common.h"
#include "libyuv/convert_from.h"

//we must define usage_exit due to include tools_common.h
void usage_exit(void) {
    exit(-1);
}

namespace impl {
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

        return AImage(std::move(buffer), {image->r_w, image->r_h}, APixelFormat::RGBA_BYTE);
    }
}

AWebmFramesFactory::AWebmFramesFactory(_<IInputStream> stream) {
    //mProcessingFuture = asyncX [this, stream = std::move(stream)]() {
//        MyWebmCallback callback(this);
//        MyWebmReader reader(std::move(stream));
//        webm::WebmParser parser;
//        parser.Feed(&callback, &reader);
    //};

    mProcessingFuture.onError([](const AException& e) {
        ALogger::err("webm") << "Error occured while decoding webm : " << e;
    });
}

AWebmFramesFactory::~AWebmFramesFactory() {
    mProcessingFuture.cancel();
    vpx_codec_destroy(mContext.ptr());
}

AImage AWebmFramesFactory::provideImage(const glm::ivec2 &size) {
    if (mFrameBuffer.isFrameAvailable() && mPlaybackStarted.time_since_epoch().count() == 0) {
        mPlaybackStarted = std::chrono::system_clock::now();
    }

    return (mFrameBuffer.extractFrame())->image;
}

bool AWebmFramesFactory::isNewImageAvailable() {
    if (mPlaybackStarted.time_since_epoch().count() == 0) {
        return mFrameBuffer.isFrameAvailable();
    }

    if (auto timecode = mFrameBuffer.nextFrameTimecode()) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mPlaybackStarted).count() >= *timecode;
    }
    static_assert(sizeof(vpx_codec_err_t) == sizeof(int));
    return false;
}

glm::ivec2 AWebmFramesFactory::getSizeHint() {
    return IImageFactory::getSizeHint();
}

void AWebmFramesFactory::onFrameLoaded(AByteBufferView buffer, int16_t timecode) {
    auto start = std::chrono::high_resolution_clock::now();
    if (auto code = vpx_codec_decode(mContext.ptr(),
                                     reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(),
                                     nullptr, 0)) {
        ALogger::warn("webm") << "Failed to decode frame, code : " << code;
        return;
    }

    vpx_codec_iter_t iter = nullptr;

    while (auto image = vpx_codec_get_frame(mContext.ptr(), &iter)) {
        try {
            mFrameBuffer.enqueueFrame({
                  .image = impl::convertToRGBA(image),
                  .timecode = timecode
            });
        }
        catch(...) {
            ALogger::warn("webm") << "Converting frame failed, skipping...";
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    ALogger::info("webm") << "TIME FOR FRAME DECODING: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void AWebmFramesFactory::onVideoTrackInfoParsed(std::string_view codecId) {
    auto decoder = get_vpx_decoder_by_name(codecId == "V_VP8" ? "vp8" : "vp9");
    if (!decoder) {
        throw AException("failed to fetch vpx decoder");
    }

    if (vpx_codec_dec_init(mContext.ptr(), decoder->codec_interface(), nullptr, 0)) {
        throw AException("failed to initialize codec context");
    }
}
