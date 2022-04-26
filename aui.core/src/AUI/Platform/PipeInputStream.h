#pragma once

#include <AUI/IO/IInputStream.h>
#include "Pipe.h"

class PipeInputStream: public IInputStream {
public:
    explicit PipeInputStream(Pipe pipe);
    ~PipeInputStream();

    size_t read(char* dst, size_t size) override;

private:
    Pipe mPipe;

#if AUI_PLATFORM_UNIX
    FILE* mFileHandle;
#endif
};
