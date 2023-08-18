//
// Created by dervisdev on 2/6/2023.
//

#pragma once

#include <cstdint>
#include <cassert>


class SDL_Audio_Base {
public:
    virtual ~SDL_Audio_Base() {}

    virtual bool release() = 0;

    virtual bool play() = 0;

    virtual bool fadeIn(int ms) = 0;

    virtual bool fadeOut(int ms) = 0;

    virtual bool stop() = 0;

    virtual bool pause() = 0;

    virtual bool resume() = 0;

    virtual bool rewind() = 0;

    virtual bool play(bool loop) {
        setLoop(loop);
        return play();
    }

    virtual bool fadeIn(int ms, bool loop) {
        setLoop(loop);
        return fadeIn(ms);
    }

    virtual bool isPlaying() = 0;

    virtual bool isPaused() = 0;

    virtual bool isStopped() {
        return !isPlaying() && !isPaused();
    }

    virtual bool isLoop() {
        return mLoop;
    }

    virtual void setLoop(bool loop) {
        mLoop = loop;
    }

    virtual void setVolume(float vol) {
        assert(("vol must be an integer between 0 and 1 inclusively", 0.f <= vol && vol <= 1.f));
        mVolume = vol;
    }

private:
    bool mLoop = false;
    float mVolume = 100;
};
