#include "Stylesheet.h"

#include <utility>
#include "AUI/Util/ATokenizer.h"
#include "AUI/IO/StringStream.h"
#include "AUI/Util/EnumUtil.h"
#include "AUI/Common/AMap.h"
#include "AUI/View/AView.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/AViewContainer.h"

#if defined(_WIN32)
#undef max
#include <dwmapi.h>
#endif

Stylesheet::PreferredStyle Stylesheet::ourPrefferedStyle = Stylesheet::PREFER_NATIVE_LOOK;

Stylesheet::Entry::Matching Stylesheet::Entry::selectorMatches(AView* view, bool forcePossibleEntries)
{
	/*
	size_t hash = reinterpret_cast<size_t>(view->getParent());

	hash ^= std::hash<Set<AString>>()(Set<AString>(view->getCssNames().begin(), view->getCssNames().end()));
	hash ^= view->isEnabled() * 71245 | view->isMouseHover() * 876126 | view->isMousePressed() * 999125;
	Map<AString, AVariant> attrs;
	view->getCustomCssAttributes(attrs);
	for (auto& attr : attrs)
	{
		hash ^= std::hash<AString>()(attr.first);
		hash ^= attr.second.hash();
	}

	if (auto c = mCache.contains(hash))
	{
		if (c->second.classes.isSubsetOf(view->getCssNames()) &&
			view->getCssNames().isSubsetOf(c->second.classes))
		{
			return c->second.result;
		}
	}
	*/
	auto cache = [&](int m) -> Matching
	{
		//mCache[hash] = CacheEntry{view->getCssNames(), m };
		return static_cast<Matching>(m);
	};
	auto targetView = view;


	int finalResult = M_MISMATCH;


	auto subSelectors = mSelector.split(',');

	for (auto& subSelector : subSelectors)
	{
	    if (subSelector == "*")
	        return M_MATCH;
		subSelector = subSelector.trim();
		try
		{
			enum
			{
				TARGET_CURRENT,
				TARGET_INDIRECT_PARENT,
			} target = TARGET_CURRENT;


			using Reverse = std::reverse_iterator<std::wstring::iterator>;
			Reverse lastMilestone(subSelector.end());
			Reverse end(subSelector.begin());

			auto it = lastMilestone;

			auto processToken = [&](const AString& token) -> int
			{
				ATokenizer p(_new<StringStream>(token));

				auto name = p.readString({ '_', '-', '.' });

				bool nameMatches = false;

				switch (target)
				{
				case TARGET_CURRENT:
					nameMatches = view->getCssNames().contains(name);
					break;
				case TARGET_INDIRECT_PARENT:
					for (; view; view = view->getParent())
					{
						nameMatches = view->getCssNames().contains(name);
						if (nameMatches)
							break;
					}
					break;
				}
				if (!view)
					return M_MISMATCH;


				if (nameMatches)
				{
					try {
						bool c = true;
						while (c) {
							switch (p.readChar())
							{
							default:
								p.reverseByte();
								c = false;
								break;

							case '[':
							{
								auto attributeName = p.readString({ '_', '-', '.' });

								AMap<AString, AVariant> attrs;

								if (forcePossibleEntries && view != targetView)
									AObject::connect(view->customCssPropertyChanged, targetView->mCssHelper,
										&ACSSHelper::onCheckPossiblyMatchCss);

								view->getCustomCssAttributes(attrs);

								switch (p.readChar())
								{
								case ']':
									// мы прочитали атрибут
									if (!forcePossibleEntries && !attrs.contains(attributeName))
										return M_MISMATCH;
									break;

								case '=':
									auto value = p.readString();
									if (value.startsWith("\"") && value.endsWith("\""))
										value = value.mid(1, value.length() - 2);
									p.readChar(); // ]

									if (!forcePossibleEntries && attrs[attributeName].toString() != value)
										return M_MISMATCH;


									break;
								}
							}

							break;

							case ':':
							{
								auto pseudoClass = p.readString();
								if (forcePossibleEntries)
								{

									if (pseudoClass == "hover")
									{
										AObject::connect(view->hoveredState, targetView->mCssHelper,
											&ACSSHelper::onCheckPossiblyMatchCss);
									}
									else if (pseudoClass == "active")
									{
										AObject::connect(view->pressedState, targetView->mCssHelper,
											&ACSSHelper::onCheckPossiblyMatchCss);
									}
									else if (pseudoClass == "focus")
									{
										AObject::connect(view->focusState, targetView->mCssHelper,
											&ACSSHelper::onCheckPossiblyMatchCss);
									}
								}
								else {
									if (pseudoClass == "hover")
									{
										if (!(view->isMouseHover() || view->isMousePressed()))
											return M_MISMATCH;
									}
									else if (pseudoClass == "active")
									{
										if (!(view->isMouseHover() && view->isMousePressed()))
											return M_MISMATCH;
									}
									else if (pseudoClass == "focus")
									{
										if (!(view->hasFocus()))
											return M_MISMATCH;
									}
								}
							}
							break;
							}
						}
					} catch (...)
					{
					}
					if (forcePossibleEntries)
						return M_POSSIBLY_MATCH;
					return M_MATCH;
				}

				return M_MISMATCH;
			};
			int result = M_MISMATCH;

			for (; it != end; ++it)
			{
				switch (*it)
				{
				case ' ':
					break;
				default:
					continue;
				}

				AString token = { it.base(), lastMilestone.base() };

				result = processToken(token);
				if (result == 0)
					throw 0;

				view = reinterpret_cast<AView*>(view->getParent());
				if (!view)
					return cache(M_MISMATCH);

				target = TARGET_INDIRECT_PARENT;

				lastMilestone = it;
			}
			if (lastMilestone != it)
			{
				AString token = {it.base(), lastMilestone.base()};

				result = processToken(token);
			}
			finalResult = (glm::max)(finalResult, result);
			if (result == M_MATCH)
				return cache(result);
		}
		catch (int)
		{
		}
	}
	return cache(finalResult);
}

Stylesheet::Stylesheet()
{
#if defined(_WIN32)
	DWORD c = 0;
	BOOL blending;
	DwmGetColorizationColor(&c, &blending);
	c |= 0xff000000;
	AColor osThemeColor = AColor::fromAARRGGBB(static_cast<unsigned>(c));
	float readability = osThemeColor.readabilityOfForegroundColor(0xffffffff);
	if (readability < 0.3f)  {
	    osThemeColor = osThemeColor.darker(1.f - readability * 0.5f);
	}
    setVariable("OS_THEME_COLOR", osThemeColor.toString());
#else
    setVariable("OS_THEME_COLOR", "#3e3e3e");
#endif
    if (ourPrefferedStyle == PREFER_NATIVE_LOOK) {
#if defined(_WIN32)
        load(AUrl(":win/style.less").open());
#else
        load(AUrl(":uni/style.less").open());
#endif
    } else {
        setVariable("COLOR_ACCENT", AColor::fromAARRGGBB(0xffff2147u).toString());
        load(AUrl(":uni/style.less").open());
    }
}

void Stylesheet::load(const AString& css) noexcept
{
	load(_new<StringStream>(css));
}

void Stylesheet::load(const _<IInputStream>& css) noexcept {
    mGlobalCache.load(*this, css);
}

void Stylesheet::Cache::load(Stylesheet& ss, const _<IInputStream>& css, bool skipSelector) noexcept
{
    assert(css);
	ATokenizer p(css);
	AMap<AString, Entry::Property::Type> items = {
		{"background", Entry::Property::T_BACKGROUND},
		{"background-color", Entry::Property::T_BACKGROUND_COLOR},
		{"background-size", Entry::Property::T_BACKGROUND_SIZE},
		{"background-repeat", Entry::Property::T_BACKGROUND_REPEAT},
		{"background-effect", Entry::Property::T_BACKGROUND_EFFECT},
		{"color", Entry::Property::T_COLOR},
		{"text-align", Entry::Property::T_TEXT_ALIGN},
		{"transition", Entry::Property::T_TRANSITION},
		{"cursor", Entry::Property::T_CURSOR},
		{"overflow", Entry::Property::T_OVERFLOW},
		{"box-shadow", Entry::Property::T_BOX_SHADOW},
		{"vertical-align", Entry::Property::T_VERTICAL_ALIGN},

		{"font-family", Entry::Property::T_FONT_FAMILY},
		{"font-size", Entry::Property::T_FONT_SIZE},
		{"text-transform", Entry::Property::T_TEXT_TRANSFORM},

		{"border", Entry::Property::T_BORDER},
		{"border-bottom", Entry::Property::T_BORDER_BOTTOM},
		{"border-radius", Entry::Property::T_BORDER_RADIUS},
		{"margin", Entry::Property::T_MARGIN},
		{"padding", Entry::Property::T_PADDING},

		{"min-width", Entry::Property::T_WIDTH_MIN},
		{"min-height", Entry::Property::T_HEIGHT_MIN},
		{"max-width", Entry::Property::T_WIDTH_MAX},
		{"max-height", Entry::Property::T_HEIGHT_MAX},
		{"width", Entry::Property::T_WIDTH},
		{"height", Entry::Property::T_HEIGHT},

		{"-aui-spacing", Entry::Property::T_AUI_SPACING},
		{"-aui-offset", Entry::Property::T_AUI_OFFSET},
		{"-aui-scale", Entry::Property::T_AUI_SCALE},
		{"-aui-font-rendering", Entry::Property::T_AUI_FONT_RENDERING},
        {"-aui-background-overlay", Entry::Property::T_AUI_BACKGROUND_OVERLAY},
	};

    char c;
    AString selector;

	auto beginReadingBody = [&]() {
        // entry
        AMap<Entry::Property::Type, _<Entry::Property>> properties;
        AString propertyName;

        for (;;)
        {
            c = p.readChar();
            switch (c)
            {
                case '}':
                    if (!properties.empty())
                    {
                        mEntries << _new<Entry>(selector, properties);
                        properties.clear();
                    }
                    selector.clear();
                    throw 0;

                case '\t':
                case '\n':
                case '\r':
                case ' ':
                    break;
                case ':':
                    if (!propertyName.empty())
                    {
                        auto propertyType = items.find(propertyName);
                        propertyName.clear();
                        if (propertyType == items.end())
                            continue;

                        AString propertyValue;

                        auto insertProperty = [&]()
                        {
                            if (!propertyValue.empty())
                            {
                                auto args = propertyValue.split(' ').noEmptyStrings();

                                for (auto& a : args)
                                {
                                    if (a.startsWith("$"))
                                    {
                                        a = ss.getVariable(a.mid(1));
                                    }
                                }

                                properties[propertyType->second] = _new<Entry::Property>
                                        (propertyType->second, args);
                            }
                            mEntries << _new<Entry>(selector, properties);
                        };

                        for (;;) {
                            try {
                                c = p.readChar();
                            } catch (const IOException&) {
                                c = 0;
                            }
                            if (c == '}' || c == 0) {
                                insertProperty();
                                selector.clear();
                                properties.clear();
                                throw 0;
                            }
                            if (c == ';') {
                                break;
                            }
                            if (c != '\n')
                                propertyValue += c;
                        }
                        insertProperty();
                    }
                    break;
                default:
                    if (isalnum(c) || c == '-' || c == '_')
                    {
                        propertyName += c;
                    }
            }
        }
	};

	if (skipSelector) {
	    selector = "*";
	    try {
            beginReadingBody();
        } catch (...) {}
	} else {
        for (;;) {
            try {
                c = p.readChar();
                if (isalnum(c) ||
                    c == ',' ||
                    c == '.' ||
                    c == '#' ||
                    c == ' ' ||
                    c == ':' ||
                    c == '[' ||
                    c == ']' ||
                    c == '=' ||
                    c == '_' ||
                    c == '*') {
                    // selector
                    selector += c + p.readString({'-', '_'});
                } else if (c == '{') {
                    selector = selector.trim();
                    beginReadingBody();
                }
            }
            catch (int) {
            }
            catch (const IOException&) {
                break;
            }
            catch (...) {
            }
        }
    }
}

void Stylesheet::setPreferredStyle(Stylesheet::PreferredStyle style) {
    ourPrefferedStyle = style;
}

Stylesheet& Stylesheet::inst() {
    static Stylesheet s;
    return s;
}
/*
void Stylesheet::invalidateCache()
{
	for (auto& e : mEntries)
	{
		e->mCache.clear();
	}
}
*/
