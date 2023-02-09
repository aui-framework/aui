//
// Created by dervisdev on 2/6/2023.
//

#pragma once

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned char byte;

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          if(p) {delete p; p = 0;}
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)    if(p) {delete[] p; p = 0;}
#endif


#define TYPE_WAV        1
#define TYPE_MP3        2
#define TYPE_OGG        3

#define MAX_VOLUME      128


class SDL_Audio_Base {
public:
    SDL_Audio_Base()
            : m_loop(0), m_volume(128) {}

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
        set_loop(loop);
        return play();
    }

    virtual bool fadeIn(int ms, bool loop) {
        set_loop(loop);
        return fadeIn(ms);
    }

    virtual bool is_playing() = 0;

    virtual bool is_paused() = 0;

    virtual bool is_stopped() {
        return !is_playing() && !is_paused();
    }

    virtual bool is_loop() {
        return 0 != m_loop;
    }

    virtual void set_volume(int vol) {
        m_volume = vol;
    }

    virtual void set_loop(bool loop) {
        m_loop = (true == loop) ? -1 : 0;
    }


protected:
    int m_loop;
    int m_volume;
};


template<typename T>
class _singleton {
public:
    _singleton() {
    }

    ~_singleton() {
    }

    static T* get_instance() {
        static T instance;
        return &instance;
    }
};
//  declare the object is singleton pattern
#define SINGLETON_OBJECT(obj)           friend class _singleton<obj>
