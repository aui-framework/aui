# Custom sound sources

`IAudioPlayer::fromStream` plays any [ISoundInputStream]. Use it for sound that is synthesized, arrives over the
network, or is decoded by a library of your choice. For files, see [audio-playback].

## Implementing a sound stream

An [ISoundInputStream] is an [IInputStream] that produces raw interleaved PCM samples, with one additional method,
`info`, which tells the format of these samples with [AAudioFormat]. Any combination of mono/stereo, sample rate and
sample format (`I16`, `I24`, `I32`, `F32`) is accepted; the player converts it to the format of the audio device.

`read` fills `dst` with up to `size` bytes and returns the number of bytes written. Return a whole number of frames (one
sample of every channel); returning 0 means the end of the sound.

The following stream generates a sine tone of one second:

```cpp
class SineStream: public ISoundInputStream {
public:
    size_t read(char* dst, size_t size) override {
        auto samples = std::span(reinterpret_cast<int16_t*>(dst), size / sizeof(int16_t));
        size_t written = 0;
        // stereo: both channels of a frame get the same value
        for (size_t i = 0; i + 1 < samples.size() && mFramesLeft > 0; i += 2, --mFramesLeft) {
            auto value = static_cast<int16_t>(std::sin(mPhase) * 8000.f);
            mPhase += 2.f * glm::pi<float>() * 440.f / 44100.f;
            samples[i] = samples[i + 1] = value;
            written += 2 * sizeof(int16_t);
        }
        return written;
    }

    AAudioFormat info() override {
        return AAudioFormat {
            .channelCount = AChannelFormat::STEREO,
            .sampleRate = 44100,
            .sampleFormat = ASampleFormat::I16,
        };
    }

private:
    float mPhase = 0.f;
    size_t mFramesLeft = 44100;
};
```

Play it as usual:

```cpp
auto player = IAudioPlayer::fromStream(_new<SineStream>());
player->play();
```

## Limitations

!!! warning

    A player created from a stream plays once. After the sound has ended or `stop` was called, the stream is released,
    and the player can't be started again or looped, because this requires reopening the source by its url. Create a new
    player for each playback instead. A looping `fromUrl` player doesn't have this limitation.

!!! note

    `read` is called from the audio thread of the platform backend. It must be fast and must not block; do the heavy
    work (network, decoding) elsewhere and keep a buffer.

## Decoding packets

[ISoundPipe] is an [ISoundInputStream] that is also an `IOutputStream`: compressed packets are written to it, decoded
samples are read from it. `AOpusSoundPipe` (Opus packets, 48000 Hz stereo) is an example, which is handy for voice
chat style applications. The decoded output is played the same way, via `IAudioPlayer::fromStream`.
