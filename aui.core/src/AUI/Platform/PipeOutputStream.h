#pragma once

#include <AUI/IO/IOutputStream.h>
#include "Pipe.h"

class PipeOutputStream: public IOutputStream {
public:
    explicit PipeOutputStream(Pipe pipe) : mPipe(std::move(pipe)) {}

    void write(const char* src, size_t size) override;

private:
    Pipe mPipe;
};
