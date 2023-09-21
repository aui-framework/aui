#pragma once

#include "AUI/Video/IVideoParser.h"

namespace webm {
    struct TrackEntry;
}

class WebmParser : public IVideoParser {
public:
    explicit WebmParser(_<IInputStream> source);

    void setSource(_<IInputStream> source);

    void run() override;

private:
    _<IInputStream> mSource;


    friend class MyWebmCallback;
    //functions for MyWebmCallback
    void onVideoTrackParsed(const webm::TrackEntry& info);

    void onAudioTrackParsed(const webm::TrackEntry& info);

    void onVideoFrameParsed(AByteBuffer buffer, int64_t timecode);

    void onAudioFrameParsed(AByteBuffer buffer, int64_t timecode);

    void onFinished();


};
