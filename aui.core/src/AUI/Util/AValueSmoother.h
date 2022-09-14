#pragma once

template<typename T = float>
class AValueSmoother {
public:
    AValueSmoother(const T& current, const T& smoothK = 0.5) noexcept
            : mCurrent(current),
              mSmoothK(smoothK),
              mStart(current)
    {
    }

    AValueSmoother() noexcept
            : mCurrent(0),
              mSmoothK(0.5),
              mStart(0)
    {
    }

    T nextValue(T value) noexcept
    {
        auto delta = glm::abs(mCurrent - value);
        auto toAdd = (value - mCurrent) * mSmoothK;
        if (delta < glm::abs(toAdd)) { // avoid chatter
            mCurrent = value;
        } else {
            mCurrent += toAdd;
        }
        return mCurrent;
    }


    void setCurrent(T value) noexcept
    {
        mCurrent = value;
    }

    T getCurrent() noexcept
    {
        return mCurrent;
    }

    void setK(T k) noexcept
    {
        mSmoothK = k;
    }
    T getK() noexcept
    {
        return mSmoothK;
    }

    void reset() noexcept
    {
        mCurrent = mStart;
    }

private:
    T mCurrent;
    T mSmoothK;
    T mStart;
};