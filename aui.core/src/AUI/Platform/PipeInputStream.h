#pragma once

#include <AUI/IO/IInputStream.h>
#include "Pipe.h"

class PipeInputStream: public IInputStream {
public:
    explicit PipeInputStream(Pipe pipe) : mPipe(std::move(pipe)) {}

    size_t read(char* dst, size_t size) override;

private:
    Pipe mPipe;
};
