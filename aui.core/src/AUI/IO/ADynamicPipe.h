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


