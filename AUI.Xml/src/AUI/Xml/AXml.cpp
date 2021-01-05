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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include "AXml.h"

#include <functional>


#include "AXmlParseError.h"
#include "AUI/Util/ATokenizer.h"

void AXml::read(_<IInputStream> is, _<IXmlDocumentVisitor> visitor)
{
	ATokenizer p(is);

	char c;

	auto throwUnexceptedCharacter = [&]()
	{
		throw AXmlParseError(
			std::string("Unexpected character '") + p.getLastCharacter() + "' at " + std::to_string(p.getRow()) + ":" +
			std::to_string(p.getRow()));
	};
	try
	{
		for (; (c = p.readChar());)
		{
			switch (c)
			{
			case '\r':
			case '\n':
			case '\t':
			case ' ':
				break;
			case '<':
				// ����� ��������
				switch (p.readChar())
				{
				case '?':
					{
						// ���������
						auto headerVisitor = visitor->visitHeader();

						p.readString(); // ��������� "xml"

						for (; (c = p.readChar()) != '>';)
						{
							switch (c)
							{
							case '?':
								break;

							case ' ':
								break;

							default:
								{
									p.reverseByte();
									auto attrName = p.readString();
									bool attrWithNoValue = false;
									if (!attrName.empty())
									{
										for (; (c = p.readChar()) != '=';)
										{
											if (!isblank(c))
											{
												if (headerVisitor)
													headerVisitor->visitAttribute(attrName, {});
												attrWithNoValue = true;
												break;
											}
										}
									}
									if (!attrWithNoValue)
									{
										AString value;
										[&]()
										{
											for (; (c = p.readChar());)
											{
												switch (c)
												{
												case '\'':
													value = p.readStringUntilUnescaped('\'');
													return;

												case '"':
													value = p.readStringUntilUnescaped('"');
													return;
												case ' ':
													break;

												default:
													throwUnexceptedCharacter();
												}
											}
										}();

										if (headerVisitor)
											headerVisitor->visitAttribute(attrName, value);
									}
								}
							}
						}
					}
					break;
				default:
					// ���������� ��������
					{
						p.reverseByte();
						auto entityName = p.readString();

						std::function<void(_<IXmlEntityVisitor>)> handleEntity;
						handleEntity = [&](_<IXmlEntityVisitor> entityVisitor)
						{
							bool endFlag = false;
							for (; (c = p.readChar()) != '>';)
							{
								switch (c)
								{
								case '/':
									endFlag = true;
									break;

								case '\r':
								case '\n':
								case ' ':
									break;

								default:
									{
										p.reverseByte();
										auto attrName = p.readString({':'});
										if (attrName.empty())
										{
											p.readChar();
											continue;
										}
										bool attrWithNoValue = false;
										for (; (c = p.readChar()) != '=';)
										{
											if (!isblank(c))
											{
												if (entityVisitor)
													entityVisitor->visitAttribute(attrName, {});
												attrWithNoValue = true;
												break;
											}
										}
										if (!attrWithNoValue)
										{
											AString value;
											[&]()
											{
												for (;;)
												{
													switch (c = p.readChar())
													{
													case '\'':
														value = p.readStringUntilUnescaped('\'');
														return;

													case '"':
														value = p.readStringUntilUnescaped('"');
														return;
													case ' ':
														break;

													default:
														throwUnexceptedCharacter();
													}
												}
											}();

											if (entityVisitor)
												entityVisitor->visitAttribute(attrName, value);
										}
									}
								}
							}
							if (!endFlag)
							{
								[&]()
								{
									for (;;)
									{
										c = p.readChar();
										switch (c)
										{
										case ' ':
											break;
										case '\t':
											break;
										case '<':
											c = p.readChar();
											if (c == '/')
											{
												// ����� ���� ��������.
												p.readString();

												if (p.readChar() != '>')
													throwUnexceptedCharacter();

												return;
											}
											// ������ ����� ��������.
											{
												p.reverseByte();
												auto entityName = p.readString();
												handleEntity(entityVisitor->visitEntity(entityName));
											}

										default:
											{
												auto str = p.readStringUntilUnescaped('<');
												p.reverseByte();
												if (entityVisitor)
													entityVisitor->visitTextEntity(str);
											}
										}
									}
								}();
							}
						};
						handleEntity(visitor->visitEntity(entityName));
					}
				}

				break;

			default:
				throwUnexceptedCharacter();
			}
		}
	}
	catch (const EOFException&)
	{
	}
}
