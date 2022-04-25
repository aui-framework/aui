#pragma once

#include <AUI/IO/IOutputStream.h>
#include "Pipe.h"

class PipeOutputStream: public IOutputStream {
public:
    explicit PipeOutputStream(Pipe pipe);
    ~PipeOutputStream();

    void write(const char* src, size_t size) override;

private:
    Pipe mPipe;

#if AUI_PLATFORM_UNIX
    FILE* mFileHandle;
#endif
};
