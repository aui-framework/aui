#pragma once

#include "AUI/Image/IImageFactory.h"

class IAudioPlayer;

//TODO think more about factories and linking with ADrawableView

class IVideoFactory : public IImageFactory {
public:
    const _<IAudioPlayer>& relatedPlayer() const {
        return mPlayer;
    }

protected:
    _<IAudioPlayer> mPlayer;
};
