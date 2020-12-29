//
// Created by alex2 on 16.11.2020.
//

#pragma once


#include "IInputStream.h"

/**
 * \brief Это поток, который пропустит через себя максимум определённое количество байт - дальше он будет возвращать eof
 *        (конец потока)
 */
class API_AUI_CORE ALimitedInputStream: public IInputStream {
private:
    _<IInputStream> mInputStream;
    size_t mLimit;

public:
    ALimitedInputStream(const _<IInputStream>& inputStream, size_t limit) : mInputStream(inputStream), mLimit(limit) {}
    virtual ~ALimitedInputStream() = default;

    int read(char* dst, int size) override;
};


