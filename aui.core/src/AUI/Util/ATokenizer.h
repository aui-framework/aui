/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <utility>
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AColor.h"
#include "AUI/Common/ASet.h"

class API_AUI_CORE ATokenizer
{
private:
	_<IInputStream> mInput;

	char mLastByte;
	bool mReverse = false;

	int mRow = 1;
	int mColumn = 1;
	
public:
	ATokenizer(_<IInputStream> inputStream)
		: mInput(std::move(inputStream))
	{
	}

	explicit ATokenizer(const AString& fromString);

	/**
	 * \brief Reads string while isalnum == true.
	 * \return read string
	 */
	AString readString();

	/**
	 * \brief Reads string while pred(char) == true.
	 * \return read string
	 */
    template<typename Callable>
	AString readStringWhile(Callable pred) {
        AString res;
        char c;

        try
        {
            for (;;) {
                c = readChar();
                if (pred(c))
                {
                    res << c;
                }
                else
                {
                    reverseByte();
                    return res;
                }
            }
        } catch (...)
        {

        }
        return res;
	}

	/**
	 * \brief Reads <code>n</code> symbols.
	 * \return read string
	 */
	AString readString(int n);

	/**
	 * \brief Reads string while isalnum == true and characters contain in <code>applicableChars</code>.
	 * \return read string
	 */
	AString readString(const ASet<char>& applicableChars);


	/**
	 * \brief Reads character.
	 * \return read character
	 */
	char readChar();

	/**
	 * \brief Rejects the last read byte and return it into the "stream". Applicable for parsing algorithms.
	 */
	void reverseByte();

	/**
	 * \brief Reads float point number.
	 * \return read float point number
	 */
	float readFloat();


	/**
	 * \brief Reads integer number.
	 * \return read integer number
	 */
	int readInt();

	/**
	 * \brief Reads unsigned integer number.
	 * \return read unsigned integer number
	 */
	unsigned readUInt();

	/**
	 * \brief Reads unsigned integer number.
	 * \return read unsigned integer number + bool isHex
	 */
	std::tuple<unsigned, bool> readUIntX();

	/**
	 * \return last read byte. Applicable with <code>ATokenizer::reverseByte()</code>
	 */
	char getLastCharacter()
	{
		return mLastByte;
	}

	/**
	 * \brief Get row counter value. Applicable for error reporting
	 * \return row counter
	 */
	int getRow() const
	{
		return mRow;
	}

    /**
     * \brief Get column counter value. Applicable for error reporting
     * \return column counter
     */
	int getColumn() const
	{
		return mColumn;
	}

	/**
	 * \brief Skips character until unescaped c.
	 * \param c character to read until to
	 */
	void skipUntilUnescaped(char c);

	/**
	 * \brief Skips character until c.
	 * \param c character to read until to
	 */
	void skipUntil(char c);


    /**
     * \brief Reads string until unescaped c.
     * \param c character to read until to
     * \return read string
     */
	AString readStringUntilUnescaped(char c);

    /**
     * \brief Reads string until unescaped c.
     * \param characters characters to read until to
     * \return read string
     */
	AString readStringUntilUnescaped(const ASet<char>& characters);

    /**
     * \brief Reads string until unescaped c.
     * \param out read string
     * \param c character to read until to
     */
    void readStringUntilUnescaped(std::string& out, char c);

    /**
     * \brief Reads string until unescaped c.
     * \param out read string
     * \param characters characters to read until to
     */
    void readStringUntilUnescaped(std::string& out, const ASet<char>& characters);

	/**
	 * \brief reads 2 floats divided by any symbol.
	 * \return vec2
	 */
	glm::vec2 readVec2();
};
