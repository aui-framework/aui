#pragma once
#include "IOutputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/SharedPtr.h"

class API_AUI_CORE ByteBufferOutputStream: public IOutputStream
{
private:
    _<AByteBuffer> mBuffer;

public:
    ByteBufferOutputStream(const _<AByteBuffer>& buffer)
            : mBuffer(buffer)
    {
    }

    int write(const char* src, int size) override;
};
