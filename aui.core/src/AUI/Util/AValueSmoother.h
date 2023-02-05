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