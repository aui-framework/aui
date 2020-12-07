#pragma once

#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AMap.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Views.h"

class AView;
class AViewContainer;
class AString;

class API_AUI_VIEWS Stylesheet
{
public:
    enum PreferredStyle {
        /**
         * \brief В этом режиме приложение старается выглядеть максимально нативно, насколько это возможно.
         */
        PREFER_NATIVE_LOOK,

        /**
         * \brief В этом режиме приложение старается выглядеть максимально одинаково на разных платформах.
         */
        PREFER_UNIVERSAL_LOOK,
    };

	class Entry
	{
		friend class Stylesheet;
	public:
		class Property
		{
		public:
			enum Type
			{
				// Common
				T_BACKGROUND,
				T_BACKGROUND_COLOR,
				T_BACKGROUND_SIZE,
				T_BACKGROUND_REPEAT,
				T_BACKGROUND_EFFECT,
				T_COLOR,
				T_TEXT_ALIGN,
				T_TRANSITION,
				T_CURSOR,
				T_OVERFLOW,
				T_BOX_SHADOW,
				T_VERTICAL_ALIGN,

				// Fonts
				T_FONT_FAMILY,
				T_FONT_SIZE,
				T_TEXT_TRANSFORM,

				// Fields
				T_BORDER_RADIUS,
				T_BORDER,
				T_BORDER_BOTTOM,
				T_MARGIN,
				T_PADDING,

				// Mins
				T_WIDTH_MIN,
				T_HEIGHT_MIN,
				// Maxs
				T_WIDTH_MAX,
				T_HEIGHT_MAX,
				// Size
				T_WIDTH,
				T_HEIGHT,

				// AUI
				T_AUI_SPACING,
                T_AUI_OFFSET,
                T_AUI_SCALE,
				T_AUI_FONT_RENDERING,
                T_AUI_BACKGROUND_OVERLAY,
			};
			
		private:
			Type mType;
			AStringVector mArgs;
			
		public:
			Property(Type type, const AStringVector& args)
				: mType(type),
				  mArgs(args)
			{
			}
			Type getType() const
			{
				return mType;
			}

			const AStringVector& getArgs() const
			{
				return mArgs;
			}
		};

		enum Matching
		{
			M_MISMATCH = 0,
			M_POSSIBLY_MATCH,
			M_MATCH
		};
	private:
		AString mSelector;
		AMap<Property::Type, _<Property>> mProperties;

		struct CacheEntry
		{
			ADeque<AString> classes;
			Matching result;
		};

		//Map<size_t, CacheEntry> mCache;
		
	public:

		Entry(AString selector, const AMap<Property::Type, _<Property>>& properties)
			: mSelector(std::move(selector)),
			  mProperties(std::move(properties))
		{
		}

		const AString& getSelector() const
		{
			return mSelector;
		}

		const AMap<Property::Type, _<Property>>& getSheet() const
		{
			return mProperties;
		}

		Matching selectorMatches(AView* view, bool forcePossibleEntries = true);
	};

	class Cache {
    private:
        ADeque<_<Entry>> mEntries;

    public:
	    void clear() {
	        mEntries.clear();
	    }

        [[nodiscard]] const ADeque<_<Entry>>& getEntries() const {
            return mEntries;
        }
        void load(Stylesheet& ss, const _<IInputStream>& css, bool skipSelector = false) noexcept;
    };

private:
	AMap<AString, AString> mVariables;
	static PreferredStyle ourPrefferedStyle;
	Cache mGlobalCache;

public:
	Stylesheet();

	static Stylesheet& inst();

	static void setPreferredStyle(PreferredStyle style);

	[[nodiscard]] const ADeque<_<Entry>>& getEntries() const noexcept
	{
		return mGlobalCache.getEntries();
	}

	void load(const AString& css) noexcept;
	void load(const _<IInputStream>& css) noexcept;

	void setVariable(const AString& name, const AString& value)
	{
		mVariables[name] = value;
	}
	
	const AString& getVariable(const AString& name) noexcept
	{
		return mVariables[name];
	}

	//void invalidateCache();
};

using css = Stylesheet::Entry::Property::Type;