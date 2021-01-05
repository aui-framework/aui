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
	 * \brief Reads <code>n</code> symbols.
	 * \return read string
	 */
	AString readString(int n);

	/**
	 * \brief Reads string while isalnum == true and characters contains in <code>applicableChars</code>.
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
     * \brief Reads string until unescaped c.
     * \param c character to read until to
     * \return read string
     */
	AString readStringUntilUnescaped(char c);

    /**
     * \brief Reads string until unescaped c.
     * \param out read string
     * \param c character to read until to
     */
    void readStringUntilUnescaped(std::string& out, char c);

	/**
	 * \brief reads 2 floats divided by any symbol.
	 * \return vec2
	 */
	glm::vec2 readVec2();
};
