#pragma once

#include "AUI/Audio/ISoundPipe.h"
#include "AUI/IO/ADynamicPipe.h"
#include "AUI/Util/APimpl.h"

typedef struct vorbis_info vorbis_info;
typedef struct vorbis_comment vorbis_comment;
typedef struct vorbis_dsp_state vorbis_dsp_state;
typedef struct vorbis_block vorbis_block;

class AVorbisSoundPipe : public ISoundPipe {
public:
    AVorbisSoundPipe();

    void write(const char *src, size_t size) override;

    size_t read(char* dst, size_t) override;

    AAudioFormat info() override;

    [[nodiscard]]
    bool isLastWriteSuccessful() const {
        return mLastWriteSuccessful;
    }

    static constexpr ASampleFormat SAMPLE_FORMAT = ASampleFormat::I16;

private:
    enum class WaitingTarget {
        ID_HEADER,
        COMMENT,
        INFO,
        AUDIO_PACKET
    };

    WaitingTarget mWaitingTarget;
    aui::fast_pimpl<vorbis_info, 8 * std::max(sizeof(long), sizeof(void*))> mInfo;
    aui::fast_pimpl<vorbis_comment, 4 * sizeof(void*)> mComment;
    aui::fast_pimpl<vorbis_dsp_state, 160> mState;
    aui::fast_pimpl<vorbis_block, 200> mBlock;

    ADynamicPipe mDecodedSamples;
    bool mLastWriteSuccessful = true;
};
