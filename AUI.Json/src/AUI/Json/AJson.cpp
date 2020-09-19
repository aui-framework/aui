#include "AJson.h"
#include "AUI/Util/Tokenizer.h"
#include "JsonArray.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonException.h"

AJsonElement AJson::read(_<IInputStream> is)
{
	Tokenizer t(is);

	std::function<_<IJsonElement>()> read;

	auto unexpectedCharacter = [&]()
	{
		throw JsonException(
			AString("unexpected character ") + t.getLastCharacter() + " at " + t.getRow() + ":" + t.
			getColumn());
	};
	
	read = [&]() -> _<IJsonElement>
	{
		_<IJsonElement> result;

		for (;;)
		{
			switch (t.readChar())
			{
			case '[':
				result = _new<JsonArray>();
				do
				{
					if (!isspace(t.getLastCharacter()))
						result->asArray() << AJsonElement(read());
				}
				while (t.readChar() != ']');

				return result;
			case '{':
				result = _new<JsonObject>();
				while (t.readChar() != '}')
				{
					if (!isspace(t.getLastCharacter()))
					{
						if (t.getLastCharacter() == '\"')
						{
							AString key = t.readStringUntilUnescaped('\"');

							for (char c = 0; ;)
							{
								c = t.readChar();
								if (c == ':')
									break;
								if (!isspace(c))
									unexpectedCharacter();
							}
							result->asObject()[key] = AJsonElement(read());
						}
						else if (t.getLastCharacter() != ',')
						{
							unexpectedCharacter();
						}
					}
				}

				return result;

			case '\"':
				result = _new<JsonValue>(t.readStringUntilUnescaped('\"'));
				return result;
			}

			if (isdigit(t.getLastCharacter()))
			{
				t.reverseByte();
				return _new<JsonValue>(t.readInt());
			}
		}
		assert(!isspace(t.getLastCharacter()));
		return result;
	};

	return AJsonElement(read());
}

void API_AUI_JSON AJson::write(_<IOutputStream> os, const AJsonElement& json) {
    json.serialize(os);
}
