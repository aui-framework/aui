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