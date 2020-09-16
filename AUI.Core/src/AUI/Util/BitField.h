#pragma once


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
     * \param flag флаг
     * \return true, если флаг был поднят и сброшен этим вызовом
     */
    bool checkAndTake(T flag) {
        if (mStorage & flag) {
            mStorage &= ~flag;
            return true;
        }
        return false;
    }

    /**
     * \brief Определяет, есть ли флаг в BitField и поднимает флаг, если его нет.
     * \param flag флаг
     * \return true, если флаг был опущен и поднят этим вызовом
     */
    bool checkAndPut(T flag)
    {
        if (mStorage & flag)
        {
            return false;
        }
        mStorage |= flag;
        return true;
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