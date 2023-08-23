//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "ISoundStream.h"
#include "vorbis/vorbisfile.h"
#include "AUI/Audio/Stream/FileStream.h"
#include "AUI/IO/AFileInputStream.h"

class OggSoundStream: public ISoundStream {
public:
    explicit OggSoundStream(_<IFileStream> fis);

    ~OggSoundStream() override;

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

    void rewind() override;

    static _<OggSoundStream> load(_<IFileStream> is);

private:
    _<IFileStream> mFis;
    OggVorbis_File mVorbisFile;
};

//TODO replace FileStream with abstract stream with seek() and tell() methods

