//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "vorbis/vorbisfile.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Audio/ISoundInputStream.h"

class AUrl;
class ISeekableInputStream;


/**
 * @brief Sound stream for OGG format
 * @ingroup audio
 */
class API_AUI_AUDIO AOggSoundStream: public ISoundInputStream {
public:
    explicit AOggSoundStream(_<ISeekableInputStream> fis);

    ~AOggSoundStream() override;

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

    void rewind() override;

    static _<AOggSoundStream> load(_<ISeekableInputStream> is);
    static _<AOggSoundStream> fromUrl(const AUrl& url);

private:
    _<ISeekableInputStream> mStream;
    OggVorbis_File mVorbisFile;
};
