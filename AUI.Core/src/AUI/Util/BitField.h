#pragma once


/**
 * \brief Bit field implementation.
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
     * \brief Sets flag.
     * \param flag flag
     * \return this
     */
    BitField& operator<<(T flag) {
        mStorage |= flag;
        return *this;
    }

    /**
     * \brief Resets flag.
     * \param flag flag
     * \return this
     */
    BitField& operator>>(T flag) {
        mStorage &= ~flag;
        return *this;
    }

    /**
     * \brief Determines whether flag set or not and resets flag.
     * \param flag flag
     * \return true if flag was set
     */
    bool checkAndSet(T flag) {
        if (mStorage & flag) {
            mStorage &= ~flag;
            return true;
        }
        return false;
    }

    /**
     * \brief Determines whether flag set or not and sets flag.
     * \param flag flag
     * \return true if flag was reset
     */
    bool checkAndReset(T flag)
    {
        if (mStorage & flag)
        {
            return false;
        }
        mStorage |= flag;
        return true;
    }

    /**
     * \brief Determines whether flag(s) set or not.
     * \param flags flag(s)
     * \return true if flag(s) set
     * \note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool check(T flags) const {
        return (mStorage & flags) == flags;
    }

    /**
     * \brief Determines whether flag(s) set or not.
     * \param flags flag(s)
     * \return true if flag(s) set
     * \note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool operator&(T flags) const {
        return check(flags);
    }
};