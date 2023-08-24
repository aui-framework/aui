//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "ISoundStream.h"
#include "vorbis/vorbisfile.h"
#include "AUI/IO/AFileInputStream.h"

class AUrl;
class ISeekableInputStream;

class OggSoundStream: public ISoundStream {
public:
    explicit OggSoundStream(_<ISeekableInputStream> fis);

    ~OggSoundStream() override;

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

    void rewind() override;

    static _<OggSoundStream> load(_<ISeekableInputStream> is);
    static _<OggSoundStream> fromUrl(const AUrl& url);

private:
    _<ISeekableInputStream> mStream;
    OggVorbis_File mVorbisFile;
};
