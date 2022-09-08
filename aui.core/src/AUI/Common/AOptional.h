#pragma once

#include <cstdint>
#include <cassert>
#include <utility>
#include <optional>
#include <stdexcept>


/**
 * @brief Utility wrapper implementing the stack-allocated (fast) optional idiom.
 * @ingroup useful_templates
 */
template<typename T>
class AOptional {
public:
    AOptional() noexcept = default;
    AOptional(std::nullopt_t) noexcept {}

    AOptional(T&& rhs) noexcept {
        operator=(std::forward<T>(rhs));
    }

    AOptional(const T& rhs) {
        operator=(rhs);
    }

    AOptional(const AOptional& rhs) {
        operator=(rhs);
    }

    AOptional(AOptional&& rhs) noexcept {
        operator=(std::move(rhs));
    }

    ~AOptional() {
        if (mInitialized) ptrUnsafe()->~T();
    }


    [[nodiscard]]
    bool hasValue() const noexcept {
        return mInitialized;
    }

    operator bool() const noexcept {
        return hasValue();
    }

    template<typename... Args>
    AOptional<T>& emplace(Args&&... args) {
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(std::forward<Args>(args)...);
        mInitialized = true;
        return *this;
    }

    AOptional<T>& operator=(std::nullopt_t) noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
            mInitialized = false;
        }
        return *this;
    }

    AOptional<T>& operator=(const AOptional& rhs) noexcept {
        if (rhs) {
            operator=(rhs.value());
        }
        return *this;
    }

    AOptional<T>& operator=(AOptional&& rhs) noexcept {
        if (rhs) {
            operator=(std::move(rhs.value()));
            rhs.reset();
        }
        return *this;
    }

    AOptional<T>& operator=(T&& rhs) noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(std::forward<T>(rhs));
        mInitialized = true;
        return *this;
    }

    AOptional<T>& operator=(const T& rhs) {
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(rhs);
        mInitialized = true;
        return *this;
    }

    [[nodiscard]]
    T& value() noexcept {
        assert(("optional is empty", mInitialized));
        return reinterpret_cast<T&>(mStorage);
    }

    [[nodiscard]]
    const T& value() const noexcept {
        assert(("optional is empty", mInitialized));
        return reinterpret_cast<const T&>(mStorage);
    }

    [[nodiscard]]
    T* ptr() noexcept {
        return &value();
    }

    [[nodiscard]]
    const T* ptr() const noexcept {
        return &value();
    }

    [[nodiscard]]
    T* operator->() noexcept {
        return ptr();
    }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return ptr();
    }

    [[nodiscard]]
    T& operator*() noexcept {
        return value();
    }

    [[nodiscard]]
    const T& operator*() const noexcept {
        return value();
    }

    void reset() noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
            mInitialized = false;
        }
    }

    /**
     * @brief valueOr
     * @tparam F same as T or invocable returning T or invokable throwing a exception
     * @param alternative
     * @return
     */
    template<typename F>
    T valueOr(F&& alternative) const {
        if (mInitialized) {
            return value();
        }
        constexpr bool isSame = std::is_constructible_v<T, F>;
        constexpr bool isInvocable = std::is_invocable_v<F>;

        static_assert(isSame || isInvocable, "F is neither same as T nor invokable returning T nor invokable throwing a exception");

        if constexpr (isSame) {
            return std::forward<F>(alternative);
        } else if constexpr(isInvocable) {
            if constexpr (std::is_same_v<std::invoke_result_t<F>, void>) {
                alternative();
                assert(("should not have reached here", false));
                throw std::runtime_error("should not have reached here"); // stub exception
            } else {
                return alternative();
            }
        }
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> mStorage;
    bool mInitialized = false;

    [[nodiscard]]
    T* ptrUnsafe() noexcept {
        return &valueUnsafe();
    }

    [[nodiscard]]
    T& valueUnsafe() noexcept {
        return reinterpret_cast<T&>(mStorage);
    }

};
