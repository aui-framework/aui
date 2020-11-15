//
// Created by alex2 on 15.11.2020.
//

#pragma once


#include "IInputStream.h"
#include "IOutputStream.h"

class API_AUI_CORE APipe: public IInputStream, public IOutputStream {
private:
    char mCircularBuffer[0x10000];
    uint16_t mReaderPos = 0;
    uint16_t mWriterPos = 0;

    AMutex mMutex;
    AConditionVariable mConditionVariable;

    bool mClosed = false;

public:
    APipe();
    virtual ~APipe();

    int read(char* dst, int size) override;
    int write(const char* src, int size) override;

    size_t available();

    void close();

};


