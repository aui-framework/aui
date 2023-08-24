#include "ASampleCommitter.h"
#include "AUI/Audio/Mixer/details/ASampleConsumer.h"
#include "AUI/Audio/AAudioPlayer.h"

ASampleCommitter::ASampleCommitter(_<ISoundStream> stream, PlaybackConfig config) :
            mSoundStream(std::move(stream)), mConfig(config) {
    mFormat = mSoundStream->info();
}

size_t ASampleCommitter::requestSoundData(char* dst, size_t size) {
    ASampleConsumer consumer(dst, dst + size);
    switch (mFormat.bitsPerSample) {
        case 16:
            consumer.commitAllSamples<SampleFormat::I16>(mSoundStream);
        break;
        case 24:
            consumer.commitAllSamples<SampleFormat::I24>(mSoundStream);
        break;
    }
    return consumer.writtenSize();
}

bool ASampleCommitter::requestRewind() {
    mSoundStream->rewind();
    return true;
}

PlaybackConfig ASampleCommitter::getConfig() {
    return mConfig;
}
