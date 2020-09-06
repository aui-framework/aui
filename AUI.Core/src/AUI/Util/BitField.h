#pragma once

#include <cstdint>

/**
 * \brief Битовое поле.
 */
template <typename T = uint32_t>
class BitField {
private:
    T mStorage;

public:
    BitField(T storage = static_cast<T>(0)) : mStorage(storage) {}

    operator T() {
        return mStorage;
    }

    T& storage() {
        return mStorage;
    }

    /**
     * \brief Запихнуть флаг в битовое поле.
     * \param flag Флаг
     * \return this
     */
    BitField& operator<<(T flag) {
        mStorage |= flag;
        return *this;
    }

    /**
     * \brief Убрать флаг из битового поля.
     * \param flag Флаг
     * \return this
     */
    BitField& operator>>(T flag) {
        mStorage &= ~flag;
        return *this;
    }

    /**
     * \brief Определяет, есть ли флаг в BitField и сбрасывает флаг, если он есть.
     * \param flag Флаг
     * \return true, если флаг поднят в битовом поле
     */
    bool checkAndTake(T flag) {
        if (mStorage & flag) {
            mStorage &= ~flag;
            return true;
        }
        return false;
    }

    /**
     * \brief Проверить, поднят ли флаг в битовом поле.
     * \param flags Флаг
     * \return true, если флаг поднят
     */
    bool check(T flags) const {
        return mStorage & flags;
    }

    bool operator&(T flags) const {
        return mStorage & flags;
    }
};