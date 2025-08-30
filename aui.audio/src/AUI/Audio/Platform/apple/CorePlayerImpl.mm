#if AUI_PLATFORM_IOS
#import <UIKit/UIApplication.h>
#endif
#import <AVFoundation/AVFoundation.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

#include "AUI/Common/AQueue.h"
#include "CoreAudioPlayer.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/Platform/RequestedAudioFormat.h"


static constexpr auto LOG_TAG = "CoreAudio";

namespace {
static AAudioMixer& mixer() {
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
        strdesc.mBitsPerChannel = aui::audio::bytesPerSample(aui::audio::platform::requested_sample_format) * 8;
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
    
    [[nodiscard]]
    const _<AThread> thread() const noexcept {
        return mThread;
    }
    
    CoreAudioInstance(const CoreAudioInstance&) = delete;
    
    ~CoreAudioInstance() {
        thread()->interrupt();
        thread()->join();
    }
    
    void enqueueIfNot() {
        while (!mEmptyBuffers.empty()) {
            auto buffer = mEmptyBuffers.front();
            buffer->mAudioDataByteSize = mixer().readSoundData({(std::byte*)buffer->mAudioData, buffer->mAudioDataBytesCapacity});
            if (buffer->mAudioDataByteSize == 0) {
                break;
            }
            AudioQueueEnqueueBuffer(mAudioQueue, buffer, 0, nullptr);
            
            mEmptyBuffers.pop();
        }
    }
    
private:
    _<AThread> mThread;
    AudioQueueRef mAudioQueue = nullptr;
    AVector<AudioQueueBufferRef> mAudioQueueBuffers;
    AQueue<AudioQueueBufferRef> mEmptyBuffers;
    
    static void myQueueOutputCallback(void* userdata, AudioQueueRef aq, AudioQueueBufferRef buffer) {
        auto thiz = reinterpret_cast<CoreAudioInstance*>(userdata);
        buffer->mAudioDataByteSize = mixer().readSoundData({(std::byte*)buffer->mAudioData, buffer->mAudioDataBytesCapacity});
        if (buffer->mAudioDataByteSize > 0) {
            AudioQueueEnqueueBuffer(thiz->mAudioQueue, buffer, 0, nullptr);
            thiz->enqueueIfNot();
        } else {
            thiz->mEmptyBuffers << buffer;
        }
    }
};

static CoreAudioInstance& coreAudio() {
    mixer(); // init order
    static CoreAudioInstance p;
    return p;
}
}


void CoreAudioPlayer::playImpl() {
    coreAudio().thread()->enqueue([self = aui::ptr::shared_from_this(this)] {
        self->initializeIfNeeded();
        ::mixer().addSoundSource(self);
        coreAudio().enqueueIfNot();
    });
}

void CoreAudioPlayer::pauseImpl() {
    coreAudio().thread()->enqueue([self = aui::ptr::shared_from_this(this)] {
        ::mixer().removeSoundSource(self);
    });
}

void CoreAudioPlayer::stopImpl() {
    coreAudio().thread()->enqueue([self = aui::ptr::shared_from_this(this)] {
        ::mixer().removeSoundSource(self);
        self->reset();
    });
}

void CoreAudioPlayer::onVolumeSet() {

}

void CoreAudioPlayer::onLoopSet() {

}
