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
