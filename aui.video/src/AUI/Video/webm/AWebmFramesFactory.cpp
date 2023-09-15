#include "AWebmFramesFactory.h"
#include "AUI/Common/AQueue.h"
#include "webm/webm_parser.h"
#include "AUI/Thread/AThreadPool.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vpx_codec.h"

class MyWebmCallback : public webm::Callback {
public:
    void onFrameLoaded(std::function<void(AByteBuffer, int16_t)>&& callback) {
        mOnFrameLoaded = std::move(callback);
    }

    webm::Status OnBlockBegin(const webm::ElementMetadata& metadata, const webm::Block& block, webm::Action* action) override {
        if (!mOnFrameLoaded) {
            return webm::Callback::OnBlockBegin(metadata, block, action);
        }

        *action = webm::Action::kRead;
        mLastTimecode = block.timecode;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnSimpleBlockBegin(const webm::ElementMetadata& metadata, const webm::SimpleBlock& block, webm::Action* action) override {
        return OnBlockBegin(metadata, block, action);
    }

    webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytesRemaining) override {
        if (!mOnFrameLoaded) {
            return webm::Callback::OnFrame(metadata, reader, bytesRemaining);
        }

        if (mBuffer.capacity() < metadata.size) {
            mBuffer.reserve(metadata.size);
        }

        uint64_t actuallyRead;
        reader->Read(*bytesRemaining, reinterpret_cast<uint8_t*>(mBuffer.end()), &actuallyRead);
        mBuffer.increaseSize(actuallyRead);
        (*bytesRemaining) -= actuallyRead;
        if (*bytesRemaining == 0) {
            mOnFrameLoaded(std::move(mBuffer), mLastTimecode);
            return webm::Status(webm::Status::kOkCompleted);
        }

        return webm::Status(webm::Status::kOkPartial);
    }

private:
    int16_t mLastTimecode = 0;
    AByteBuffer mBuffer;

    std::function<void(AByteBuffer, int16_t)> mOnFrameLoaded;
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
        uint8_t stub[0x4000];
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
    mCallback = _new<MyWebmCallback>();
    mParser = _new<webm::WebmParser>();
    mParser->Feed(mCallback.get(), mReader.get());
}

AImage AWebmFramesFactory::provideImage(const glm::ivec2 &size) {
    return (mFrameBuffer.extractFrame())->image;
}

bool AWebmFramesFactory::isNewImageAvailable() {
    if (mPlaybackStarted.time_since_epoch().count() == 0) {
        return mFrameBuffer.isFrameAvailable();
    }

    if (auto timecode = mFrameBuffer.currentTimecode()) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - mPlaybackStarted).count() >= *timecode;
    }

    return false;
}

glm::ivec2 AWebmFramesFactory::getSizeHint() {
    return IImageFactory::getSizeHint();
}
