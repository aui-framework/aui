// AUI Framework - Declarative UI toolkit for modern C++17
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

//
// Created by Alex2772 on 11/17/2021.
//
#pragma once

/**
 * Simple Math's fraction consisting of two integers, allowing to perform high-precision calculations without FPU.
 */
class AFraction {
private:
    int mNumerator;
    int mDenominator;

public:
    AFraction(int numerator, int denominator) : mNumerator(numerator), mDenominator(denominator) {}
    AFraction(int number): mNumerator(number), mDenominator(1) {}

    AFraction& operator+=(int number) {
        mNumerator += number * mDenominator;
        return *this;
    }

    AFraction operator+(int number) const {
        auto copy = *this;
        copy += number;
        return copy;
    }

    AFraction& operator-=(int number) {
        mNumerator -= number * mDenominator;
        return *this;
    }

    AFraction operator-(int number) const {
        auto copy = *this;
        copy -= number;
        return copy;
    }

    AFraction& operator*=(int number) {
        mNumerator *= number;
        return *this;
    }

    AFraction operator*(int number) const {
        auto copy = *this;
        copy *= number;
        return copy;
    }

    AFraction& operator/=(int number) {
        mDenominator *= number;
        return *this;
    }

    AFraction operator/(int number) const {
        auto copy = *this;
        copy /= number;
        return copy;
    }

    [[nodiscard]]
    int toInt() const {
        return mNumerator / mDenominator;
    }

    [[nodiscard]]
    float toFloat() const {
        return float(mNumerator) / float(mDenominator);
    }
    [[nodiscard]]
    double toDouble() const {
        return double(mNumerator) / double(mDenominator);
    }
};