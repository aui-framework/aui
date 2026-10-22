# Playing sounds

This page describes how to play sound files with `aui::audio` and control the playback. To play audio that is generated
or decoded by your code, see [audio-custom-sources].

## Playing a file

Create an [IAudioPlayer] with `IAudioPlayer::fromUrl` and call `play`. The url can point to a file, to an
[aui-assets] resource (the `:` prefix) or to any other location [AUrl] can open. WAV and Ogg Vorbis are detected
automatically.

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp audio_from_url -->

`play` returns immediately; the sound is mixed and sent to the audio device in the background. Keep the returned
pointer alive as long as you want the sound to be played, or at least until it is finished.

!!! note

    The sound is opened when the player is created, so a missing file or an unsupported format throws an exception
    right in `fromUrl` (`aui::audio::ABadFormatException` for the latter).

## Pause, stop and resume

`play` starts or resumes playback, `pause` stops it keeping the position, and `stop` stops it and discards the
position, so the next `play` starts from the beginning. The current state is available as `playbackStatus`.

Since these are plain methods, they connect directly to signals:

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp audio_controls -->

## Volume

`setVolume` takes an integer from 0 (silence) to 256 (original volume); the scale is linear. It can be changed at any
time, including while the sound is playing.

```cpp
player->setVolume(128); // half of the original volume
```

## Looping

With `setLoop(true)` the sound starts over when it ends instead of finishing.

```cpp
auto music = IAudioPlayer::fromUrl(":sound/music.ogg");
music->setLoop(true);
music->play();
```

## Playing several sounds at once

Create a player per sound and start them independently. Their output is mixed by [AAudioMixer]; each player has its own
volume. Sounds with different sample rates, channel counts or formats can be mixed together, as everything is converted
to the format of the audio device.

!!! note

    At most `AAudioMixer::MAX_PLAYER_COUNT` (100) players can play simultaneously.

## Knowing when the sound ends

The `finished` signal is emitted on the thread which created the player when the sound ended (not looping) or when
reading it failed.

```cpp
auto player = IAudioPlayer::fromUrl(":sound/jingle.ogg");
connect(player->finished, [] {
    ALogger::info("Sound") << "done";
});
player->play();
```

## Sound on UI events

The `ass::Sound` style rule plays a sound when a style rule is applied, so a button click sound can be declared along
with the rest of the style:

<!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp audio_ass_sound -->

See [ass] for details on styling.

## Platforms

| Platform      | Backend    |
|---------------|------------|
| Linux         | PulseAudio |
| Windows       | DirectSound |
| macOS, iOS    | CoreAudio  |
| Android       | Oboe       |
| others        | stub (silent) |

The player requests stereo 44100 Hz output from the backend.
