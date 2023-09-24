#include "AVorbisSoundPipe.h"
#include "vorbis/codec.h"
#include "AUI/Logging/ALogger.h"
#include "os.h"

static constexpr auto LOG_TAG = "vorbis";

AVorbisSoundPipe::AVorbisSoundPipe() {
    vorbis_comment_init(mComment.ptr());
    vorbis_info_init(mInfo.ptr());
}

void AVorbisSoundPipe::write(const char *src, size_t size) {
    ARaiiHelper helper = [data = std::string(src, size), src, size]() {
        for (size_t i = 0; i < size; i++) {
            assert(data[i] == src[i]);
        }
    };
    ogg_packet packet;
    packet.packet = reinterpret_cast<unsigned char*>(const_cast<char*>(src));
    packet.bytes = static_cast<long>(size);
    packet.b_o_s = mWaitingTarget != WaitingTarget::AUDIO_PACKET;

    switch (mWaitingTarget) {
        case WaitingTarget::ID_HEADER:
            if (vorbis_synthesis_headerin(mInfo.ptr(), mComment.ptr(), &packet)) {
                ALogger::warn(LOG_TAG) << "Failed to parse ID header";
                mLastWriteSuccessful = false;
                return;
            }

            mWaitingTarget = WaitingTarget::COMMENT;
            break;

        case WaitingTarget::COMMENT:
            if (vorbis_synthesis_headerin(mInfo.ptr(), mComment.ptr(), &packet)) {
                ALogger::warn(LOG_TAG) << "Failed to parse ID comment header";
                mLastWriteSuccessful = false;
                return;
            }

            mWaitingTarget = WaitingTarget::INFO;
            break;

        case WaitingTarget::INFO:
            if (vorbis_synthesis_headerin(mInfo.ptr(), mComment.ptr(), &packet)) {
                ALogger::warn(LOG_TAG) << "Failed to parse info header";
                mLastWriteSuccessful = false;
                return;
            }

            vorbis_synthesis_init(mState.ptr(), mInfo.ptr());
            vorbis_block_init(mState.ptr(), mBlock.ptr());
            mWaitingTarget = WaitingTarget::AUDIO_PACKET;
            break;

        case WaitingTarget::AUDIO_PACKET: {
            if (auto code = vorbis_synthesis(mBlock.ptr(), &packet)) {
                ALogger::warn(LOG_TAG) << "Failed to decode audio packet: vorbis_synthesis() returned " << code;
                mLastWriteSuccessful = false;
                return;
            }

            if (auto code = vorbis_synthesis_blockin(mState.ptr(), mBlock.ptr())) {
                ALogger::warn(LOG_TAG) << "Failed to decode audio packet: vorbis_synthesis_blockin() returned " << code;
                mLastWriteSuccessful = false;
                return;
            }

            float** pcm = nullptr;
            auto sampleCount = vorbis_synthesis_pcmout(mState.ptr(), &pcm);
            for (int sampleNum = 0; sampleNum < sampleCount; sampleNum++) {
                for (int channel = 0; channel < mInfo->channels; channel++) {
                    auto sample = static_cast<int16_t>(vorbis_ftoi(
                            glm::clamp(32768.f * pcm[channel][sampleNum], -32768.f, 32767.f)));
                    mDecodedSamples.write(reinterpret_cast<char*>(&sample), sizeof(sample));
                }
            }

            vorbis_synthesis_read(mState.ptr(), sampleCount);
            break;
        }

    }

    mLastWriteSuccessful = true;
}

size_t AVorbisSoundPipe::read(char *dst, size_t size) {
    return mDecodedSamples.read(dst, size);
}

AAudioFormat AVorbisSoundPipe::info() {
    return {
        .channelCount = static_cast<AChannelFormat>(mInfo->channels),
        .sampleRate = static_cast<uint32_t>(mInfo->rate),
        .sampleFormat = SAMPLE_FORMAT
    };
}
