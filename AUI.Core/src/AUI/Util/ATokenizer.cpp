#include "ATokenizer.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/ASet.h"
#include "AUI/IO/StringStream.h"

ATokenizer::ATokenizer(const AString& fromString):
	mInput(_new<StringStream>(fromString))
{
}

AString ATokenizer::readString()
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
AString ATokenizer::readString(const ASet<char>& applicableChars)
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

char ATokenizer::readChar()
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

void ATokenizer::reverseByte()
{
	mReverse = true;
}

float ATokenizer::readFloat()
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
			case '-':
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

int ATokenizer::readInt()
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

void ATokenizer::skipUntilUnescaped(char c) {
    for (char current; (current = readChar()) != c;)
    {
        if (current == '\\')
        {
            readChar();
        }
    }
}

AString ATokenizer::readStringUntilUnescaped(char c)
{
	std::string result;
	for (char current; (current = readChar()) != c;)
	{
		if (current == '\\')
		{
			result += '\\';
			result += readChar();
		} else
		{
			result += current;
		}
	}
	return result;
}

glm::vec2 ATokenizer::readVec2()
{
	glm::vec2 result;
	result.x = readFloat();
	readChar();
	result.y = readFloat();
	return result;
}

AString ATokenizer::readString(int n) {
    AString result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        result += readChar();
    }
    return result;
}
