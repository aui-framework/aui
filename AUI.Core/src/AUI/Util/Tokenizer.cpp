#include "Tokenizer.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/Set.h"
#include "AUI/IO/StringStream.h"

Tokenizer::Tokenizer(const AString& fromString):
	mInput(_new<StringStream>(fromString))
{
}

AString Tokenizer::readString()
{
	AString res;
	char c;

	try
	{
		for (;;) {
			c = readChar();
			if (isalnum(c))
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
AString Tokenizer::readString(const Set<char>& applicableChars)
{
	AString res;
	res.reserve(128);
	char c;

	try
	{
		for (;;) {
			c = readChar();
			if (isalnum(c) || applicableChars.find(c) != applicableChars.end())
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

char Tokenizer::readChar()
{
	if (mReverse) {
		mReverse = false;
		return mLastByte;
	}
	*mInput >> mLastByte;
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

void Tokenizer::reverseByte()
{
	mReverse = true;
}

float Tokenizer::readFloat()
{
	AString tmp;
	try {
		bool dot = false;
		char c;
		for (;;)
		{
			c = readChar();
			switch (c)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				tmp += c;
				break;
			case '.':
				if (!dot)
				{
					tmp += c;
					break;
				}
			default:
				reverseByte();
				return tmp.toFloat();
			}
		}
	} catch (...) {}
	return tmp.toFloat();
}

int Tokenizer::readInt()
{
	AString tmp;
	try {
		char c;
		for (;;)
		{
			c = readChar();
			switch (c)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				tmp += c;
				break;
			default:
				reverseByte();
				return tmp.toInt();
			}
		}
	}
	catch (...) {}
	return tmp.toInt();
}

AString Tokenizer::readStringUntilUnescaped(char c)
{
	std::string result;
	for (char current; (current = readChar()) != c;)
	{
		if (current == '\\')
		{
			result += readChar();
		} else
		{
			result += current;
		}
	}
	return result;
}

glm::vec2 Tokenizer::readVec2()
{
	glm::vec2 result;
	result.x = readFloat();
	readChar();
	result.y = readFloat();
	return result;
}
