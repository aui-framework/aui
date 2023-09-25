#pragma once

#include "AUI/Common/AQueue.h"
#include "AUI/Video/AVideoFrame.h"
#include "AUI/Video/AVideoInfo.h"
#include "AUI/Video/AAudioInfo.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundPipe.h"

class IVideoParser;
class IFrameDecoder;
class AVideoFrame;
class AEncodedFrame;

class AAsyncVideoProcessor : public AObject {
public:
    AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder);

    void run();

    _<AAsyncVideoProcessor> sharedPtr() {
        return _cast<AAsyncVideoProcessor>(AObject::sharedPtr());
    }

    bool hasFinished() {
        return mHasFinished;
    }

    AOptional<AAudioInfo> audioInfo();

    AOptional<AVideoInfo> videoInfo();

    AOptional<AVideoFrame> nextVideoFrame();

    AOptional<AEncodedFrame> nextAudioFrame();

    const AOptional<AAudioInfo>& audioInfo() const;

    const AOptional<AVideoInfo>& videoInfo() const;

    void playAudio() {
        mPlayer->play(); //TODO temporary solution
    }

private:
    static constexpr size_t VIDEO_BUFFER_CAPACITY = 10;

    template<typename T, size_t capacity>
    class ReadyFramesQueue {
    public:
        AOptional<T> pop() {
            std::unique_lock lock(mSync);
            if (mReadyFrames.empty()) {
                return std::nullopt;
            }

            T frame = std::forward<T>(mReadyFrames.front());
            mReadyFrames.pop();
            mCV.notify_all();
            return {std::forward<T>(frame)};
        }

        [[nodiscard]]
        const T& front() const {
            return mReadyFrames.front();
        }

        void push(T frame) {
            std::unique_lock lock(mSync);
            if constexpr (capacity > 0) {
                if (mReadyFrames.size() >= capacity) {
                    mCV.wait(lock);
                }
            }

            mReadyFrames << std::forward<T>(frame);
        }

    private:
        AConditionVariable mCV;
        AMutex mSync;
        AQueue<T> mReadyFrames;
    };

    ReadyFramesQueue<AVideoFrame, VIDEO_BUFFER_CAPACITY> mReadyVideoFrames;
    ReadyFramesQueue<AEncodedFrame, 0> mReadyAudioFrames;
    _<IVideoParser> mParser;
    _<IFrameDecoder> mDecoder;
    _<AThread> mParserThread;
    _<AThread> mDecoderThread;
    _<IAudioPlayer> mPlayer;
    _<ISoundPipe> mSoundSource;

    AOptional<AVideoInfo> mVideoInfo;
    AOptional<AAudioInfo> mAudioInfo;

    bool mHasFinished = false;

    void setupCallbacks();
};
