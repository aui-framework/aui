#include "WebmParser.h"
#include "webm/webm_parser.h"

class MyWebmCallback : public webm::Callback {
public:
    explicit MyWebmCallback(WebmParser* parser) : mParser(parser) {
        assert(parser != nullptr);
    }

    webm::Status OnClusterBegin(const webm::ElementMetadata &metadata, const webm::Cluster &cluster, webm::Action *action) override {
        mClusterTimecode = cluster.timecode.value();
        *action = webm::Action::kRead;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnBlockBegin(const webm::ElementMetadata& metadata, const webm::Block& block, webm::Action* action) override {
        mIsVideoFrame = block.track_number == mVideoTrackNumber;
        mIsAudioFrame = block.track_number == mAudioTrackNumber;
        *action = webm::Action::kRead;
        mBlockTimecode = block.timecode;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnSimpleBlockBegin(const webm::ElementMetadata& metadata, const webm::SimpleBlock& block, webm::Action* action) override {
        return OnBlockBegin(metadata, block, action);
    }

    webm::Status OnTrackEntry(const webm::ElementMetadata &metadata, const webm::TrackEntry& trackEntry) override {
        if (trackEntry.track_type.value() == webm::TrackType::kVideo) {
            mParser->onVideoTrackParsed(trackEntry);
            mVideoTrackNumber = trackEntry.track_number.value();
        }

        if (trackEntry.track_type.value() == webm::TrackType::kAudio) {
            mParser->onAudioTrackParsed(trackEntry);
            mAudioTrackNumber = trackEntry.track_number.value();
        }

        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytesRemaining) override {
        if (!mIsVideoFrame && !mIsAudioFrame) {
            return webm::Callback::OnFrame(metadata, reader, bytesRemaining);
        }

        while (*bytesRemaining > 0) {
            mEncodedFrameBuffer.ensureReserved(*bytesRemaining);
            uint64_t actuallyRead;
            auto status = reader->Read(*bytesRemaining, reinterpret_cast<uint8_t*>(mEncodedFrameBuffer.end()), &actuallyRead);
            if (status.code == webm::Status::kEndOfFile) {
                break;
            }

            mEncodedFrameBuffer.increaseSize(actuallyRead);
            (*bytesRemaining) -= actuallyRead;
            if (*bytesRemaining == 0) {
                //we've got full coded frame
                if (mIsVideoFrame) {
                    mParser->onVideoFrameParsed(std::move(mEncodedFrameBuffer), mBlockTimecode + mClusterTimecode);
                }
                else if (mIsAudioFrame) {
                    mParser->onAudioFrameParsed(std::move(mEncodedFrameBuffer), mBlockTimecode + mClusterTimecode);
                }
                mEncodedFrameBuffer.clear();
                return webm::Status(webm::Status::kOkCompleted);
            }
        }

        return webm::Status(webm::Status::kOkPartial);
    }


    webm::Status OnSegmentEnd(const webm::ElementMetadata &metadata) override {
        mParser->onFinished();
        return webm::Status(webm::Status::kOkCompleted);
    }

private:
    bool mIsVideoFrame = false;
    bool mIsAudioFrame = false;
    int16_t mClusterTimecode = 0;
    int16_t mBlockTimecode = 0;

    uint64_t mVideoTrackNumber = -1;
    uint64_t mAudioTrackNumber = -1;

    AByteBuffer mEncodedFrameBuffer;
    WebmParser* mParser;
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
        auto status = Read(std::min(toSkip, sizeof(mStub)), mStub, actuallySkipped);
        if (status.completed_ok() && sizeof(mStub) < toSkip) {
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
    uint8_t mStub[0x4000];
};

WebmParser::WebmParser(_<IInputStream> stream) {
    setSource(std::move(stream));
}

void WebmParser::setSource(_<IInputStream> source) {
    mSource = std::move(source);
}

void WebmParser::run() {
    webm::WebmParser parser;
    MyWebmCallback callback(this);
    MyWebmReader reader(mSource);
    parser.Feed(&callback, &reader);
}

void WebmParser::onVideoTrackParsed(const webm::TrackEntry& info) {
    const auto& video = info.video.value();
    emit videoInfoParsed({
        .width = video.display_width.value(),
        .height = video.display_height.value()
    });
    if (info.codec_id.value() == "V_VP8") {
        emit videoCodecParsed(aui::video::Codec::VP8);
    }
    else if (info.codec_id.value() == "V_VP9") {
        emit videoCodecParsed(aui::video::Codec::VP9);
    }
}

void WebmParser::onAudioTrackParsed(const webm::TrackEntry &info) {
    if (info.codec_id.value() == "A_VORBIS") {
        emit audioCodecParsed(aui::audio::Codec::VORBIS);
    }
    if (info.codec_id.value() == "A_OPUS") {
        emit audioCodecParsed(aui::audio::Codec::OPUS);
    }
}

void WebmParser::onVideoFrameParsed(AByteBuffer buffer, int64_t timecode) {
    emit videoFrameParsed({
        .frameData = std::move(buffer),
        .timecode = timecode
    });
}

void WebmParser::onAudioFrameParsed(AByteBuffer buffer, int64_t timecode) {
    emit audioFrameParsed({
        .frameData = std::move(buffer),
        .timecode = timecode
    });
}

void WebmParser::onFinished() {
    emit finished();
}
