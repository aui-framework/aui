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

#include <utility>
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AColor.h"
#include "AUI/Common/ASet.h"

class API_AUI_CORE ATokenizer
{

public:
    ATokenizer(_<IInputStream> inputStream)
        : mInput(std::move(inputStream))
    {
    }

    bool isEof() const {
        return mEof;
    }

    explicit ATokenizer(const AString& fromString);

    /**
     * @brief Reads string while isalnum == true.
     * @return read string
     */
    const std::string& readString();

    /**
     * @brief Reads string while pred(char) == true.
     * @return read string
     */
    template<aui::predicate<char> Callable>
    const std::string& readStringWhile(Callable pred) {
        mTemporaryStringBuffer.clear();
        char c;

        try
        {
            for (;;) {
                c = readChar();
                if (pred(c))
                {
                    mTemporaryStringBuffer.push_back(c);
                }
                else
                {
                    reverseByte();
                    return mTemporaryStringBuffer;
                }
            }
        } catch (...)
        {

        }
        return mTemporaryStringBuffer;
    }

    /**
     * @brief Reads <code>n</code> symbols.
     * @return read string
     */
    const std::string& readString(size_t n);

    /**
     * @brief Reads string while isalnum == true and characters contain in <code>applicableChars</code>.
     * @return read string
     */
    const std::string& readString(const ASet<char>& applicableChars);


    /**
     * @brief Reads character.
     * @return read character
     */
    char readChar() {
        if (mReverse) {
            mReverse = false;
            return mLastByte;
        }

        if (mBufferRead >= mBufferEnd) {
            // read next blob
            mBufferEnd = mBuffer + mInput->read(mBuffer, sizeof(mBuffer));
            mBufferRead = mBuffer;
            if (mBufferEnd == mBufferRead) {
                throw AEOFException();
            }
        }

        mLastByte = *(mBufferRead++);

        if (mLastByte == '\n')
        {
            mRow += 1;
            mColumn = 1;
        } else
        {
            mColumn += 1;
        }
        return mLastByte;
    }

    /**
     * @brief Rejects the last read byte and return it into the "stream". Applicable for parsing algorithms.
     */
    void reverseByte();

    /**
     * @brief Reads float point number.
     * @return read float point number
     */
    float readFloat();


    /**
     * @brief Reads integer number.
     */
    int64_t readLongInt();


    /**
     * @brief Reads integer number.
     */
    int readInt();

    /**
     * @brief Reads unsigned integer number.
     */
    unsigned readUInt();

    /**
     * @brief Reads unsigned integer number + flag the read value is marked as hex (prefixed with 0x)
     */
    template<typename underlying_t>
    struct Hexable {
        underlying_t value;
        bool isHex;
    };
    Hexable<unsigned> readUIntX();

    /**
     * @return last read byte. Applicable with <code>ATokenizer::reverseByte()</code>
     */
    char getLastCharacter()
    {
        return mLastByte;
    }

    /**
     * @brief Get row counter value. Applicable for error reporting
     * @return row counter
     */
    int getRow() const
    {
        return mRow;
    }

    /**
     * @brief Get column counter value. Applicable for error reporting
     * @return column counter
     */
    int getColumn() const
    {
        return mColumn;
    }

    /**
     * @brief Skips character until unescaped c.
     * @param c character to read until to
     */
    void skipUntilUnescaped(char c);

    /**
     * @brief Skips character until c.
     * @param c character to read until to
     */
    void skipUntil(char c);


    /**
     * @brief Reads string until unescaped c.
     * @param c character to read until to
     * @return read string
     */
    const std::string& readStringUntilUnescaped(char c);

    /**
     * @brief Reads string until unescaped c.
     * @param characters characters to read until to
     * @return read string
     */
    const std::string& readStringUntilUnescaped(const ASet<char>& characters);

    /**
     * @brief Reads string until unescaped c.
     * @param out read string
     * @param c character to read until to
     */
    void readStringUntilUnescaped(std::string& out, char c);

    /**
     * @brief Reads string until unescaped c.
     * @param out read string
     * @param characters characters to read until to
     */
    void readStringUntilUnescaped(std::string& out, const ASet<char>& characters);

    /**
     * @brief reads 2 floats divided by any symbol.
     * @return vec2
     */
    glm::vec2 readVec2();


private:
    _<IInputStream> mInput;
    AString mTemporaryAStringBuffer;
    std::string mTemporaryStringBuffer;

    char mBuffer[4096];
    char* mBufferRead = nullptr;
    char* mBufferEnd = nullptr;

    char mLastByte;
    bool mReverse = false;
    bool mEof = false;

    int mRow = 1;
    int mColumn = 1;

    template<typename T>
    T readIntImpl();
};
