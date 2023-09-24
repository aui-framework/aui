#include "AAsyncVideoProcessor.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Video/IVideoParser.h"
#include "AUI/Video/IFrameDecoder.h"
#include "AUI/Thread/AEventLoop.h"


#include "AUI/Audio/Formats/vorbis/AVorbisSoundPipe.h"
#include "AUI/Audio/Formats/opus/AOpusSoundPipe.h"
#include <vorbis/codec.h>


namespace {
    //TODO move webm-specific functionality to subclsass or smth like this
    _<ISoundPipe> makeSoundSource(aui::audio::Codec codec, AByteBuffer header) {
        switch (codec) {
            case aui::audio::Codec::VORBIS: {
                if (header.data()[0] != 2) {
                    return nullptr;
                }

                size_t sizes[3];
                std::memset(sizes, 0, sizeof(sizes));
                size_t offset = 1;
                for (size_t i = 0; i < 2; i++) {
                    do {
                        if (offset >= header.size()) {
                            return nullptr;
                        }

                        sizes[i] += header.data()[offset];
                        ++offset;
                    } while(static_cast<unsigned char>(header.data()[offset - 1]) == 0xff);
                }

                if (header.size() < (offset + sizes[0] + sizes[1])) {
                    return nullptr;
                }

                sizes[2] = header.size() - (offset + sizes[0] + sizes[1]);

                auto result = _new<AVorbisSoundPipe>();
                for (unsigned long size : sizes) {
                    if (offset + size > header.size()) {
                        return nullptr;
                    }

                    result->write(header.data() + offset, size);
                    if (!result->isLastWriteSuccessful()) {
                        return nullptr;
                    }

                    offset += size;
                }

                return result;
            }
            case aui::audio::Codec::OPUS: {
                auto result = _new<AOpusSoundPipe>();
                result->write(header.data(), header.size());
                if (!result->isLastWriteSuccessful()) {
                    return nullptr;
                }

                return result;
            }
            default:
                return nullptr;
        }
    }
}

AAsyncVideoProcessor::AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder) :
            mParser(std::move(parser)), mDecoder(std::move(decoder)) {
    assert(mParser != nullptr);
}

void AAsyncVideoProcessor::run() {
    assert(("shouldn't be ran in the second time", mParserThread == nullptr));
    setupCallbacks();
    mParserThread = _new<AThread>([self = sharedPtr()]() {
         self->mParser->run();
    });
    mParserThread->start();

    mDecoderThread = _new<AThread>([self = sharedPtr()]() {
          AEventLoop loop;
          IEventLoop::Handle handle(&loop);
          AThread::current()->getCurrentEventLoop()->loop();
    });
    mDecoderThread->start();
}

AOptional<AVideoInfo> AAsyncVideoProcessor::videoInfo() {
    return mVideoInfo;
}

AOptional<AAudioInfo> AAsyncVideoProcessor::audioInfo() {
    return mAudioInfo;
}

AOptional<AVideoFrame> AAsyncVideoProcessor::nextVideoFrame() {
    return mReadyVideoFrames.pop();
}

AOptional<AEncodedFrame> AAsyncVideoProcessor::nextAudioFrame() {
    return mReadyAudioFrames.pop();
}

const AOptional<AVideoInfo>& AAsyncVideoProcessor::videoInfo() const {
    return mVideoInfo;
}

const AOptional<AAudioInfo>& AAsyncVideoProcessor::audioInfo() const {
    return mAudioInfo;
}

void AAsyncVideoProcessor::setupCallbacks() {
    AObject::connect(mParser->videoFrameParsed, [self = sharedPtr()](AEncodedFrame frame) {
        self->mDecoderThread->enqueue([self, frame = std::move(frame)]() {
            try {
                self->mReadyVideoFrames.push(self->mDecoder->decode(frame));
            }
            catch(...) { }
        });
        self->mDecoderThread->getCurrentEventLoop()->notifyProcessMessages();
    });

    AObject::connect(mParser->audioInfoParsed, [self = sharedPtr()](AAudioInfo info) {
        self->mSoundSource = makeSoundSource(info.codec, std::move(info.header));
        if (!self->mPlayer) {
            self->mPlayer = IAudioPlayer::fromSoundStream(self->mSoundSource);
            self->mPlayer->setLoop(true);
        }

        self->mAudioInfo = std::move(info);
    });

    AObject::connect(mParser->videoInfoParsed, [self = sharedPtr()](const AVideoInfo& info) {
        self->mVideoInfo = info;
        if (!self->mDecoder) {
            self->mDecoder = IFrameDecoder::fromCodec(info.codec);
        }
    });

    AObject::connect(mParser->audioFrameParsed, [self = sharedPtr()](AEncodedFrame frame) {
        //self->mReadyAudioFrames.push(std::move(frame));
        self->mSoundSource->write(frame.frameData.data(), frame.frameData.size());
    });

    AObject::connect(mParser->finished, [self = sharedPtr()]() {
        self->mHasFinished = true;
        static_cast<AEventLoop*>(self->mDecoderThread->getCurrentEventLoop())->stop();
    });
}
