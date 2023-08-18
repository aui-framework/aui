//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "ISoundStream.h"
#include "vorbis/vorbisfile.h"

#include "AUI/IO/AFileInputStream.h"

class OggSoundStream: public ISoundStream {
public:
    OggSoundStream(_<AFileInputStream> fis);

    ~OggSoundStream() override;

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

    void rewind() override;

    static _<ISoundStream> load(_<AFileInputStream> is) {
        return _new<OggSoundStream>(std::move(is));
    }

private:
    _<AFileInputStream> mFis;
    OggVorbis_File mVorbisFile;
};

//TODO replace AFileInputStream with abstract stream with seek() and tell() methods

