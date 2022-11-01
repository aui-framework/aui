#pragma once

/**
 * @brief Wraps the object with mutex, providing thread-safety layer and a runtime check.
 * @ingroup core
 */
template<typename T>
class AMutexWrapper: public aui::noncopyable {
public:
    AMutexWrapper(T value = T()) noexcept: mValue(std::move(value)) {}


    void lock() {
        mMutex.lock();

#if AUI_DEBUG
        mOwnerThread = std::this_thread::get_id();
#endif
    }

    AMutexWrapper& operator=(const T& rhs) {
        value() = rhs;
        return *this;
    }

    AMutexWrapper& operator=(T&& rhs) noexcept {
        value() = std::move(rhs);
        return *this;
    }

    template<typename U, std::enable_if_t<std::is_constructible_v<U, T>>* = 0>
    operator U() noexcept {
        return { value() };
    }

    void unlock() {
        mMutex.unlock();
#if AUI_DEBUG
        mOwnerThread = std::this_thread::get_id();
#endif
    }

    T& value() noexcept {
#if AUI_DEBUG
        assert(("AMutexWrapper should be locked by this thread in order to get access to the underlying object", mOwnerThread == std::this_thread::get_id()));
#endif
        return mValue;
    }

    T* operator->() noexcept {
        return &value();
    }

private:
    T mValue;
    AMutex mMutex;

#if AUI_DEBUG
    std::thread::id mOwnerThread;
#endif
};