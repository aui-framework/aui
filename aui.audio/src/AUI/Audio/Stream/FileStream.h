//
// Created by dervisdev on 2/6/2023.
//

#pragma once

#include <fstream>
#include <filesystem>
#include "AUI/IO/IInputStream.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Util/ABuiltinFiles.h"
#include <sstream>
#include "IStream.h"

using ByteBuffer = std::vector<std::uint8_t>;

class IFileStream: public Audio::IStream {
public:
    virtual ~IFileStream() = default;
    virtual void seek(int offset, std::ios::seekdir seekDir) = 0;
    virtual size_t tell() = 0;

    std::size_t fileSize() {
        auto current = tell();
        seek(0, std::ios::end);
        auto size = tell();
        seek(current, std::ios::beg);
        return size;
    }

    ByteBuffer toByteArray() {
        seek(0, std::ios::end);
        size_t size = tell();
        ByteBuffer buf(size);
        seek(0, std::ios::beg);
        for (auto current = buf.begin(); current != buf.end();) {
            size_t r = read((char*)&*current, buf.end() - current);
            if (r == 0) {
                assert(("bytebuffer incomplete" && false));
            }
            if (r > buf.end() - current) {
                assert(("buffer overrun! possibly heap corrupted" && false));
            }
            current += r;
        }

        return buf;
    }

    std::stringstream toStringStream() {
        std::stringstream ss;
        auto all = toByteArray();
        ss.write(reinterpret_cast<const char*>(all.data()), all.size());
        return ss;
    }

    std::string readLine() {
        std::string out;
        char c;
        while (read(&c, 1) != 0) {
            if (c == '\r' || c == '\n') {
                if (out.empty()) {
                    continue;
                } else {
                    break;
                }
            }
            out += c;
        }
        return out;
    }
};

class FileStream: public IFileStream {
private:
    AByteBufferView mRef;
    size_t mReadPos = 0;
    FileStream() {}

public:
    bool isEof() override {
        return mReadPos == mRef.size();
    }

    static std::shared_ptr<IFileStream> open(const AUrl& url) noexcept {
        auto buffer = AByteBuffer::fromStream(AUrl(url).open());
        auto data = new char[buffer.size()];
        memcpy(data, buffer.data(), buffer.size());
        auto is = AByteBufferView(data, buffer.size());
        auto fileStream = std::shared_ptr<FileStream>(new FileStream);
        fileStream->mRef = is;
        return fileStream;
    }

    ~FileStream() override = default;

    void seek(int offset, std::ios::seekdir seekDir) override {
        switch (seekDir) {
            case std::ios::beg: mReadPos = offset; break;
            case std::ios::cur: mReadPos += offset; break;
            case std::ios::end: mReadPos = mRef.size() + offset; break;
        }
    }

    size_t tell() override {
        return mReadPos;
    }

    size_t read(char* dst, size_t size) override {
        size_t toRead = glm::min(mReadPos + size, mRef.size()) - mReadPos;
        std::memcpy(dst, mRef.data() + mReadPos, toRead);
        mReadPos += toRead;
        return toRead;
    }
};