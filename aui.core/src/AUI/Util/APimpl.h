#pragma once

#include <cstdint>
#include <AUI/Traits/types.h>


namespace aui {
    /**
     * Utility wrapper implementing the stack-allocated (fast) pimpl idiom.
     * @note the following functions can be called only if <code>T</code> is a complete type:
     * <ul>
     *   <li>ctor</li>
     *   <li>dtor</li>
     *   <li>dtor</li>
     * </ul>
     *
     * @see https://youtu.be/mkPTreWiglk?t=157 (Russian)
     */
    template<typename T, std::size_t storageSize, std::size_t storageAlignment = alignof(int)>
    struct fast_pimpl {
    public:
        template<typename... Args>
        fast_pimpl(Args&&... args) {
            new (ptr()) T(std::forward<Args>(args)...);
            static_assert(storageSize >= sizeof(T), "not enough size");
            static_assert(storageAlignment == alignof(T), "alignment does not match");
        }

        fast_pimpl(const fast_pimpl& other) {
            new (ptr()) T(other.value());
        }

        fast_pimpl(fast_pimpl&& other) noexcept {
            new (ptr()) T(std::move(other.value()));
        }

        fast_pimpl& operator=(const fast_pimpl& other) {
            new (ptr()) T(other.value());
            return *this;
        }

        fast_pimpl& operator=(fast_pimpl&& other) noexcept {
            new (ptr()) T(std::move(other.value()));
            return *this;
        }


        ~fast_pimpl() {
            ptr()->~T();
        }

        [[nodiscard]]
        T& value() noexcept {
            return reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T& value() const noexcept {
            return reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T* ptr() noexcept {
            return &reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T* ptr() const noexcept {
            return &reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T* operator->() noexcept {
            return &reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T* operator->() const noexcept {
            return &reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T* operator*() noexcept {
            return &reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T* operator*() const noexcept {
            return &reinterpret_cast<const T&>(mStorage);
        }


    private:
        std::aligned_storage_t<storageSize, storageAlignment> mStorage;
    };
}