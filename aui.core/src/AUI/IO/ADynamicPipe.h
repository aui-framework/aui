// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "IInputStream.h"
#include "IOutputStream.h"
#include "AUI/Common/AQueue.h"
#include <array>

/**
 * @brief An asynchronous buffer that converts an IInputStream to IOutputStream (and otherwise).
 * @ingroup core
 * @ingroup io
 * @details
 * ADynamicPipe is similar to the unix pipes. It's slower than APipe but it's capable to store any byte count.
 *
 * Unlike APipe, ADynamicPipe never blocks the thread which can be useful in an asynchronous system.
 */
class API_AUI_CORE ADynamicPipe: public IInputStream, public IOutputStream {
public:
    ~ADynamicPipe() override = default;

    void write(const char* src, size_t size) override;
    size_t read(char* dst, size_t size) override;


    [[nodiscard]]
    bool empty() const noexcept {
        return mQueue.empty();
    }

private:
    static constexpr std::size_t FRAGMENT_CAPACITY = 0x10000;

    struct Fragment {
        std::array<std::uint8_t, FRAGMENT_CAPACITY> data;
        std::size_t writerPos = 0;
        std::size_t readerPos = 0;

        [[nodiscard]]
        std::size_t writerRemaining() const noexcept { return FRAGMENT_CAPACITY - writerPos; }

        [[nodiscard]]
        std::size_t readerRemaining() const noexcept { return writerPos - readerPos; }

        void push(const char*& src, size_t& size) {
            std::size_t sizeForCurrentFragment = glm::min(size, writerRemaining());
            std::memcpy(data.data() + writerPos, src, sizeForCurrentFragment);
            writerPos += sizeForCurrentFragment;
            src += sizeForCurrentFragment;
            size -= sizeForCurrentFragment;
        }
    };

    AQueue<Fragment> mQueue;
};


