//
// Created by alex2 on 13.11.2020.
//

#pragma once


#include "IInputStream.h"
#include <AUI/Common/ADeque.h>

class MultipleInputStream: public IInputStream {
private:
    ADeque<_<IInputStream>> mInputStreams;

public:
    explicit MultipleInputStream(const ADeque<_<IInputStream>>& inputStreams) : mInputStreams(inputStreams) {

    }

    ~MultipleInputStream() override = default;

    int read(char* dst, int size) override;
};


