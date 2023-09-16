#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIApplication.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include "AUI/Common/AQueue.h"
#include "AUI/Audio/AAudioPlayer.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Audio/ASoundResampler.h"
#include "AUI/Audio/APlayerSoundStream.h"


static constexpr auto LOG_TAG = "CoreAudio";

namespace {
static AAudioMixer& loop() {
    static AAudioMixer l;
    return l;
}

struct CoreAudioInstance {
    CoreAudioInstance(): mThread(_new<AThread>([&] {
        AThread::setName("AUI Audio");
        AudioStreamBasicDescription strdesc;
        aui::zero(strdesc);
        strdesc.mFormatID = kAudioFormatLinearPCM;
        strdesc.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
        strdesc.mChannelsPerFrame = 2;
        strdesc.mSampleRate = 44100;
        strdesc.mFramesPerPacket = 1;
        strdesc.mBitsPerChannel = 16;
        strdesc.mBytesPerFrame = strdesc.mChannelsPerFrame * strdesc.mBitsPerChannel / 8;
        strdesc.mBytesPerPacket = strdesc.mBytesPerFrame * strdesc.mFramesPerPacket;

        if (auto result = AudioQueueNewOutput(&strdesc, myQueueOutputCallback, this, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0, &mAudioQueue); result != noErr) {
            ALogger::err(LOG_TAG) << "AudioQueueNewOutput failed: " << result;
            return;
        }
        
        AudioChannelLayout layout;
        aui::zero(layout);
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        if (auto result = AudioQueueSetProperty(mAudioQueue, kAudioQueueProperty_ChannelLayout, &layout, sizeof(layout)); result != noErr) {
            ALogger::err(LOG_TAG) << "AudioQueueSetProperty kAudioQueueProperty_ChannelLayout failed: " << result;
        }
        
        constexpr auto NUM_AUDIO_BUFFERS = 4;
        for (int i = 0; i < NUM_AUDIO_BUFFERS; ++i) {
            AudioQueueBufferRef buffer;
            if (auto result = AudioQueueAllocateBuffer(mAudioQueue, 1024 * 4, &buffer); result != noErr) {
                ALogger::err(LOG_TAG) << "AudioQueueAllocateBuffer failed: " << result;
                break;
            }
            std::memset(buffer->mAudioData, 0, buffer->mAudioDataBytesCapacity);
            buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
            if (auto result = AudioQueueEnqueueBuffer(mAudioQueue, buffer, 0, nullptr); result != noErr) {
                ALogger::err(LOG_TAG) << "AudioQueueEnqueueBuffer failed: " << result;
                break;
            }
            mAudioQueueBuffers << buffer;
        }
        
        
        if (auto result = AudioQueueStart(mAudioQueue, nullptr); result != noErr) {
            ALogger::err(LOG_TAG) << "AudioQueueStart failed: " << result;
        }
        
        for (;;) {
            AThread::interruptionPoint();
            AThread::processMessages();
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, 0);
        }
    })) {
        mThread->start();
        
    }
    
    CoreAudioInstance(const CoreAudioInstance&) = delete;
    
    ~CoreAudioInstance() {
    }
    
    void enqueueIfNot() {
        mThread->enqueue([this] {
            while (!mEmptyBuffers.empty()) {
                auto buffer = mEmptyBuffers.front();
                buffer->mAudioDataByteSize = loop().readSoundData({(std::byte*)buffer->mAudioData, buffer->mAudioDataBytesCapacity});
                if (buffer->mAudioDataByteSize == 0) {
                    break;
                }
                AudioQueueEnqueueBuffer(mAudioQueue, buffer, 0, nullptr);
                
                mEmptyBuffers.pop();
            }
        });
    }
    
private:
    _<AThread> mThread;
    AudioQueueRef mAudioQueue = nullptr;
    AVector<AudioQueueBufferRef> mAudioQueueBuffers;
    AQueue<AudioQueueBufferRef> mEmptyBuffers;
    
    static void myQueueOutputCallback(void* userdata, AudioQueueRef aq, AudioQueueBufferRef buffer) {
        auto thiz = reinterpret_cast<CoreAudioInstance*>(userdata);
        buffer->mAudioDataByteSize = loop().readSoundData({(std::byte*)buffer->mAudioData, buffer->mAudioDataBytesCapacity});
        if (buffer->mAudioDataByteSize > 0) {
            AudioQueueEnqueueBuffer(thiz->mAudioQueue, buffer, 0, nullptr);
        } else {
            thiz->mEmptyBuffers << buffer;
        }
    }
};

static CoreAudioInstance& coreAudio() {
    static CoreAudioInstance p;
    return p;
}
}


AAudioPlayer::AAudioPlayer() {

}

AAudioPlayer::~AAudioPlayer() {
}

AAudioPlayer::AAudioPlayer(_<ISoundInputStream> stream) {
    setSource(std::move(stream));
}


void AAudioPlayer::playImpl() {
    assert(mResampler == nullptr);
    mResampler = _new<ASoundResampler>(_new<APlayerSoundStream>(_cast<AAudioPlayer>(sharedPtr())));
    ::loop().addSoundSource(_cast<AAudioPlayer>(sharedPtr()));
    coreAudio().enqueueIfNot();
}

void AAudioPlayer::pauseImpl() {
    assert(mResampler != nullptr);
    ::loop().removeSoundSource(_cast<AAudioPlayer>(sharedPtr()));
    mResampler.reset();
}

void AAudioPlayer::stopImpl() {
    assert(mResampler != nullptr);
    mSource->rewind();
    ::loop().removeSoundSource(_cast<AAudioPlayer>(sharedPtr()));
    mResampler.reset();
}


void AAudioPlayer::onSourceSet() {

}

void AAudioPlayer::onVolumeSet() {

}
