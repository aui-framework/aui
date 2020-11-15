//
// Created by alex2 on 15.11.2020.
//

#include "APipe.h"

using namespace std::chrono_literals;

int APipe::read(char* dst, int size) {
    std::unique_lock lock(mMutex);
    int read = 0;
    for (uint16_t i = 0; i < size; ++i, ++mReaderPos, ++read) {
        while (mWriterPos == mReaderPos) {
            if (read)
                return read;
            mConditionVariable.wait_for(lock, 100ms);
            if (mClosed) {
                if (read)
                    return read;
                return -1;
            }
        }
        dst[i] = mCircularBuffer[mReaderPos];
    }
    return read;
}

int APipe::write(const char* src, int size) {
    std::unique_lock lock(mMutex);
    if (mClosed) {
        return -1;
    }
    for (uint16_t i = 0; i < size; ++i, ++mWriterPos) {
        while (mWriterPos + 1 == mReaderPos) {
            mConditionVariable.notify_one();
            mConditionVariable.wait_for(lock, 100ms);
            if (mClosed) {
                return -1;
            }
        }
        mCircularBuffer[mWriterPos] = src[i];
    }
    mConditionVariable.notify_one();
    return size;
}

void APipe::close() {
    mClosed = true;
    std::unique_lock lock(mMutex);
    mConditionVariable.notify_all();
}

APipe::~APipe() {
    close();
}

APipe::APipe() {

}

size_t APipe::available() {
    return mWriterPos - mReaderPos;
}
