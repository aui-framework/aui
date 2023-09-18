#include "AWebmFramesFactory.h"
#include "AUI/Common/AQueue.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Logging/ALogger.h"

#include <webm/webm_parser.h>
#include <vpx/vpx_decoder.h>
#include <tools_common.h>
#include <libyuv/convert_from.h>

//we must define usage_exit due to include tools_common.h
void usage_exit(void) {
    exit(-1);
}

class MyWebmCallback : public webm::Callback {
public:
    explicit MyWebmCallback(AWebmFramesFactory* factory) : mFactory(factory) {
        assert(factory != nullptr);
    }

    webm::Status OnClusterBegin(const webm::ElementMetadata &metadata, const webm::Cluster &cluster, webm::Action *action) override {
        mClusterTimecode = cluster.timecode.value();
        *action = webm::Action::kRead;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnBlockBegin(const webm::ElementMetadata& metadata, const webm::Block& block, webm::Action* action) override {
        mIsVideoFrame = block.track_number == mVideoTrackUid;
        *action = webm::Action::kRead;
        mBlockTimecode = block.timecode;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnSimpleBlockBegin(const webm::ElementMetadata& metadata, const webm::SimpleBlock& block, webm::Action* action) override {
        return OnBlockBegin(metadata, block, action);
    }

    webm::Status OnTrackEntry(const webm::ElementMetadata &metadata, const webm::TrackEntry& trackEntry) override {
        if (trackEntry.track_type.value() == webm::TrackType::kVideo) {
            mVideoTrackUid = trackEntry.track_uid.value();
            mFactory->onVideoTrackInfoParsed( trackEntry.codec_id.value());
        }

        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytesRemaining) override {
        if (!mIsVideoFrame) {
            return webm::Callback::OnFrame(metadata, reader, bytesRemaining);
        }

        if (mEncodedFrameBuffer.capacity() < metadata.size) {
            mEncodedFrameBuffer.reserve(metadata.size);
        }

        uint64_t actuallyRead;
        reader->Read(*bytesRemaining, reinterpret_cast<uint8_t*>(mEncodedFrameBuffer.end()), &actuallyRead);
        mEncodedFrameBuffer.increaseSize(actuallyRead);
        (*bytesRemaining) -= actuallyRead;
        if (*bytesRemaining == 0) {
            mFactory->onFrameLoaded(std::move(mEncodedFrameBuffer), mClusterTimecode + mBlockTimecode);
            mEncodedFrameBuffer.clear();
            return webm::Status(webm::Status::kOkCompleted);
        }

        return webm::Status(webm::Status::kOkPartial);
    }

private:
    AWebmFramesFactory* mFactory{};
    bool mIsVideoFrame = false;
    int16_t mClusterTimecode = 0;
    int16_t mBlockTimecode = 0;

    uint64_t mVideoTrackUid = -1;
    AByteBuffer mEncodedFrameBuffer;
};

class MyWebmReader : public webm::Reader {
public:
    explicit MyWebmReader(_<IInputStream> stream) : mStream(std::move(stream)) {
    }

    webm::Status Read(std::size_t bytesToRead, std::uint8_t* buffer, std::uint64_t* actuallyRead) override {
        size_t read = mStream->read(reinterpret_cast<char*>(buffer), bytesToRead);
        (*actuallyRead) = read;
        mPos += *actuallyRead;

        if (*actuallyRead == 0) {
            return webm::Status(webm::Status::Code::kEndOfFile);
        }

        if (*actuallyRead != bytesToRead) {
            return webm::Status(webm::Status::Code::kOkPartial);
        }

        return webm::Status(webm::Status::Code::kOkCompleted);
    }

    webm::Status Skip(std::uint64_t toSkip, std::uint64_t *actuallySkipped) override {
        static uint8_t stub[0x4000];
        auto status = Read(std::min(toSkip, sizeof(stub)), stub, actuallySkipped);
        if (status.completed_ok() && sizeof(stub) < toSkip) {
            return webm::Status(webm::Status::kOkPartial);
        }

        return status;
    }

    std::uint64_t Position() const override {
        return mPos;
    }

private:
    uint64_t mPos = 0;
    _<IInputStream> mStream;
};

AWebmFramesFactory::AWebmFramesFactory(_<IInputStream> stream) {
    mReader = _new<MyWebmReader>(std::move(stream));
    mCallback = _new<MyWebmCallback>(this);
    mParser = _new<webm::WebmParser>();
    mContext = _new<vpx_codec_ctx_t>();
    ALogger::info("parser") << " Status = " << mParser->Feed(mCallback.get(), mReader.get()).code;
}

AWebmFramesFactory::~AWebmFramesFactory() {
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

    return false;
}

glm::ivec2 AWebmFramesFactory::getSizeHint() {
    return IImageFactory::getSizeHint();
}

void AWebmFramesFactory::onFrameLoaded(AByteBuffer buffer, int16_t timecode) {
    vpx_codec_decode(mContext.get(), reinterpret_cast<uint8_t*>(buffer.data()), buffer.size(), nullptr, 0);
    vpx_codec_iter_t iter = nullptr;

    while (auto image = vpx_codec_get_frame(mContext.get(), &iter)) {
        AByteBuffer convertedImage;
        convertedImage.resize(4 * image->w * image->h);

        switch (image->fmt) {
//            case VPX_IMG_FMT_YV12:
//                break;
            case VPX_IMG_FMT_I420: {
                auto code =
                libyuv::I420ToRGBA(image->planes[VPX_PLANE_Y], image->stride[VPX_PLANE_Y],
                                   image->planes[VPX_PLANE_U], image->stride[VPX_PLANE_U],
                                   image->planes[VPX_PLANE_V], image->stride[VPX_PLANE_V],
                                   reinterpret_cast<uint8_t *>(convertedImage.data()), 0,
                                   image->w, image->h);
                break;
            }
//            case VPX_IMG_FMT_I422:
//                break;
//            case VPX_IMG_FMT_I444:
//                break;
//            case VPX_IMG_FMT_I440:
//                break;
//            case VPX_IMG_FMT_NV12:
//                break;
//            case VPX_IMG_FMT_I42016:
//                break;
//            case VPX_IMG_FMT_I42216:
//                break;
//            case VPX_IMG_FMT_I44416:
//                break;
//            case VPX_IMG_FMT_I44016:
//                break;

            default:
                ALogger::err("webm") << "cannot convert image of unknown format";
            break;
        }

        ALogger::info("webm") << "enquequed frame with size " << image->w << " " << image->h;
        mFrameBuffer.enqueueFrame({
            .image = AImage(std::move(convertedImage), {image->w, image->h}, APixelFormat::RGBA_BYTE),
            .timecode = timecode
        });
    }
}

void AWebmFramesFactory::onVideoTrackInfoParsed(std::string_view codecId) {
    mDecoder = get_vpx_decoder_by_name(codecId == "V_VP8" ? "vp8" : "vp9");
    assert(mDecoder != nullptr);
    vpx_codec_dec_init(mContext.get(), mDecoder->codec_interface(), nullptr, 0);
    assert(mContext != nullptr);
}
