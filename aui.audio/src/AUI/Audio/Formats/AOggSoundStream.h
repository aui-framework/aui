//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "AUI/IO/AFileInputStream.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"

class AUrl;
class ISeekableInputStream;


struct OggVorbis_File;


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
    aui::fast_pimpl<OggVorbis_File, sizeof(void*) * (944 / 8), alignof(void*)> mVorbisFile;
};
