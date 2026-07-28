#include "PulseAudioPlayer.h"

#include "AUI/AppInfo.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Audio/AAudioMixer.h"
#include <pulse/simple.h>
#include <pulse/pulseaudio.h>

static constexpr auto LOG_TAG = "Pulseaudio";

static AAudioMixer& loop() {
    static AAudioMixer l;
    return l;
}

static _<AAbstractThread> gPulseThread;

static void stream_request_cb(pa_stream *s, size_t length, void *userdata) {
    if (gPulseThread == nullptr) {
        gPulseThread = AThread::current();
    }
    AThread::processMessages();
    pa_usec_t usec;
    int neg;
    pa_stream_get_latency(s,&usec,&neg);
    //printf("  latency %8d us %d\n",(int)usec, length);

    char *buf;

    // we need the buffer length aligned to 6 in order to avoid clicks.
    length -= length % 6;

    size_t nb = length;
    pa_stream_begin_write(s, (void**)&buf, &nb);
    loop().readSoundData({(std::byte*)buf, nb});
    pa_stream_write(s, buf, nb, NULL, 0LL, PA_SEEK_RELATIVE);
}

struct PulseAudioInstance {
    PulseAudioInstance(): mMainLoop(pa_threaded_mainloop_new()),
                          mApi(pa_threaded_mainloop_get_api(mMainLoop)),
                          mContext(pa_context_new(mApi, aui::app_info::name.toStdString().c_str())) {
        if (pa_context_connect(mContext, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
            ALogger::err(LOG_TAG) << "Failed to connect to pulseaudio";
            return;
        }
        pa_threaded_mainloop_start(mMainLoop);
        {
            pa_context_state_t state;
            do {
                state = pa_context_get_state(mContext);
                if (!PA_CONTEXT_IS_GOOD(state)) {
                    ALogger::err(LOG_TAG) << "Failed context";
                    return;
                }
            } while (state != PA_CONTEXT_READY);
        }

        ALogger::info(LOG_TAG) << "Initialized state";

        if (pa_channel_map_init_auto(&mChannelMap, 2, PA_CHANNEL_MAP_WAVEEX) == nullptr) {
            ALogger::err(LOG_TAG) << "Failed pa_channel_map_init_auto";
            return;
        }
        ALogger::info(LOG_TAG) << "Initialized channel map";

        mSampleSpec.channels = 2;
        mSampleSpec.format = PA_SAMPLE_S24LE;
        mSampleSpec.rate = 44100;
        mStream = pa_stream_new(mContext, "aui sink stream", &mSampleSpec, &mChannelMap);
        if (mStream == nullptr) {
            ALogger::err(LOG_TAG) << "Failed pa_stream_new";
            return;
        }
        ALogger::info(LOG_TAG) << "Initialized stream";

        mBufferAttr.fragsize = -1;
        mBufferAttr.prebuf = -1;
        mBufferAttr.maxlength = pa_usec_to_bytes(50'000, &mSampleSpec);
        mBufferAttr.tlength = pa_usec_to_bytes(50'000, &mSampleSpec);
        mBufferAttr.minreq = pa_usec_to_bytes(100, &mSampleSpec);

        pa_stream_set_write_callback(mStream, stream_request_cb, this);
        if (pa_stream_connect_playback(mStream, nullptr, &mBufferAttr,
                                       static_cast<pa_stream_flags_t>(PA_STREAM_INTERPOLATE_TIMING |
                                                                      PA_STREAM_ADJUST_LATENCY |
                                                                      PA_STREAM_AUTO_TIMING_UPDATE), nullptr, nullptr) < 0) {
            ALogger::info(LOG_TAG) << "Failed pa_stream_connect_playback";
            return;
        }
        ALogger::info(LOG_TAG) << "Initialized playback";


        {
            pa_stream_state_t state;
            do {
                state = pa_stream_get_state(mStream);
                if (!PA_STREAM_IS_GOOD(state)) {
                    ALogger::info(LOG_TAG) << "Failed stream";
                    return;
                }
            } while (state != PA_STREAM_READY);
        }

        while (gPulseThread == nullptr);
        ALogger::info(LOG_TAG) << "Finally initialized pulseaudio";

        mAvailable = true;
    }

    PulseAudioInstance(const PulseAudioInstance&) = delete;

    ~PulseAudioInstance() {
        pa_context_unref(mContext);
        pa_threaded_mainloop_free(mMainLoop);
    }

    pa_threaded_mainloop* mMainLoop;
    pa_mainloop_api* mApi;
    pa_context* mContext;
    bool mAvailable = false;
    pa_channel_map mChannelMap;
    pa_stream* mStream;
    pa_sample_spec mSampleSpec;
    pa_buffer_attr mBufferAttr;
};

static PulseAudioInstance& pulse() {
    static PulseAudioInstance p;
    return p;
}

void PulseAudioPlayer::playImpl() {
    pulse();
    if (gPulseThread == nullptr) {
        return;
    }
    gPulseThread->enqueue([self = aui::ptr::shared_from_this(this)]() mutable {
        self->initializeIfNeeded();
      ::loop().addSoundSource(std::move(self));
    });
    pa_threaded_mainloop_signal(pulse().mMainLoop, false);
}

void PulseAudioPlayer::pauseImpl() {
    if (gPulseThread == nullptr) {
        return;
    }
    gPulseThread->enqueue([self = aui::ptr::shared_from_this(this)] {
      ::loop().removeSoundSource(self);
    });
}

void PulseAudioPlayer::stopImpl() {
    if (gPulseThread == nullptr) {
        return;
    }
    gPulseThread->enqueue([self = aui::ptr::shared_from_this(this)] {
      ::loop().removeSoundSource(self);
      self->reset();
    });
}

void PulseAudioPlayer::onLoopSet() {

}

void PulseAudioPlayer::onVolumeSet() {

}
