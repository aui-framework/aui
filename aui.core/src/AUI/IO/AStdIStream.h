#pragma once

#include <istream>
#include "IInputStream.h"


/**
 * @brief std::istream implementation wrapping AUI's IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AStdIStream: public std::istream {
public:
    class StreamBuf: public std::streambuf  {
    public:
        StreamBuf(_<IInputStream> is);

        virtual ~StreamBuf();

    protected:
        int_type underflow() override;

    private:
        _<IInputStream> mIs;

        char mBuffer[0x1000]{};
    };

    AStdIStream(_<IInputStream> is): std::istream(new StreamBuf(std::move(is))) {

    }
    ~AStdIStream() {
        delete rdbuf();
    }
};