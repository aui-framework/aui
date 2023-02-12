//
// Created by dervisdev on 2/6/2023.
//

#pragma once

#include <fstream>
#include <vector>
#include <cassert>
#include "IStream.h"

namespace Audio {
    class IStream {
    public:
        virtual ~IStream() = default;
        virtual size_t read(char* dst, size_t size) = 0;
        virtual bool isEof() = 0;

        std::string readCharArrayString(std::size_t length) {
            std::string result;
            result.resize(length);

            auto r = read(result.data(), length);
            assert(r == length);

            if(std::size_t q = result.find('\0'); q != std::string::npos) {

                result.resize(q);
            }

            return result;
        }

        template<typename T>
        std::vector<T> readVector(std::size_t length) {
            std::vector<T> result;
            result.resize(length);
            read(reinterpret_cast<char*>(result.data()), result.size() * sizeof(T));
            return result;
        }

        template<typename T>
        T read() {
            T t;
            auto r = read(reinterpret_cast<char*>(&t), sizeof(T));
            assert(r == sizeof(T));
            return t;
        }
    };
}

