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
	 * \brief Прочитать строку.
	 *		  Читает строку, пока isalnum == true.
	 * \return строка
	 */
	AString readString();

	/**
	 * \brief Прочитать n символов.
	 * \return строка
	 */
	AString readString(int n);

	/**
	 * \brief Прочитать строку.
	 *		  Читает строку, пока isalnum == true.
	 * \return строка
	 */
	AString readString(const ASet<char>& applicableChars);


	/**
	 * \brief Прочитать символ.
	 * \return символ
	 */
	char readChar();

	/**
	 * \brief Функция для алгоритмов чтения. Означает отказаться от
	 *		  последнего байта и вернуть его обратно в поток.
	 */
	void reverseByte();

	/**
	 * \brief Прочитать число с плавающей точкой.
	 * \return число с плавающей точкой
	 */
	float readFloat();


	/**
	 * \brief Прочитать целое число.
	 * \return целое число
	 */
	int readInt();
	
	/**
	 * \return последний прочитанный символ
	 */
	char getLastCharacter()
	{
		return mLastByte;
	}
	int getRow() const
	{
		return mRow;
	}
	int getColumn() const
	{
		return mColumn;
	}

	AString readStringUntilUnescaped(char c);

	/**
	 * \brief читает 2 float'а, разделённые между собой любым символом.
	 * \return vec2.
	 */
	glm::vec2 readVec2();
};
