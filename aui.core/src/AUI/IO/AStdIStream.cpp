#include "AStdIStream.h"

AStdIStream::StreamBuf::StreamBuf(_<IInputStream> is) : mIs(std::move(is)) {
    setg(0, 0, 0);
    setp(mBuffer, mBuffer + std::size(mBuffer));
}

AStdIStream::StreamBuf::~StreamBuf() {

}

int AStdIStream::StreamBuf::underflow() {
    auto r = mIs->read(mBuffer, std::size(mBuffer));
    if (r == 0) return std::basic_ios<char>::traits_type::eof();
    setg(mBuffer, mBuffer, mBuffer + r);
    return std::basic_ios<char>::traits_type::to_int_type(*gptr());
}
