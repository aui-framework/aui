// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/Platform/AFontManager.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fontconfig/fontconfig.h>
#include <AUI/IO/APath.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include "AUI/Render/FreeType.h"

AFontManager::AFontManager() :
        mFreeType(_new<FreeType>())
{

    static struct FontConfig {
        FontConfig() {
            FcInit();

        }
        ~FontConfig() {
        }
    } fc;

    mDefaultFont = [&] {
        _<AFont> result;

        try {
            // try to determine user's font
            // /home/alex2772/.config/xsettingsd/xsettingsd.conf

            ATokenizer t(
                    _new<AFileInputStream>("/home/{}/.config/xsettingsd/xsettingsd.conf"_format(getenv("USER"))));

            // find something like
            // Gtk/FontName "System Font,  10"
            for (;;) {
                AString k = t.readStringWhile([](char c) { return isalpha(c) || c == '/'; });
                if (k == "Gtk/FontName") {
                    // found it! now scrap the font name
                    t.skipUntil('"');
                    auto fontName = t.readStringUntilUnescaped(',');

                    // done!
                    result = loadFont(AUrl::file(getPathToFont(fontName)));
                    break;
                } else {
                    // skip line
                    t.skipUntil('\n');
                }
            }

        } catch (...) {}

        if (result == nullptr) {
            // fallback to something default
            for (auto& d : {"Ubuntu", "Serif", "FreeSans", "Monospace"}) {
                try {
                    result = loadFont(AUrl::file(getPathToFont(d)));
                    ALogger::info("Using fallback font: {}"_format(d));
                    break;
                } catch (...) {}
            }
        }

        return result;
    }();
    if (mDefaultFont == nullptr) {
        // fallback to internal
        mDefaultFont = loadFont(":uni/font/Roboto.ttf");
    }
}

AString AFontManager::getPathToFont(const AString& font) {
    if (APath(font.toStdString()).isRegularFileExists())
        return font;

    // reference: qt ./qtbase/src/gui/text/unix/qfontconfigdatabase.cpp

    FcPattern *pattern = FcPatternCreate();
    auto tmp = font.toStdString();
    //FcDefaultSubstitute(pattern);
    auto os = FcObjectSetBuild (FC_FILE, nullptr);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(tmp.c_str()));
    FcPatternAddString(pattern, FC_STYLE, reinterpret_cast<const FcChar8*>("Regular"));

    struct FcFontSetWrap {
        FcFontSet* value;

        FcFontSetWrap(FcFontSet* value) : value(value) {}
        ~FcFontSetWrap() {
            if (value) {
                FcFontSetDestroy(value);
            }
        }
        operator bool() const {
            return value != nullptr;
        }
        FcFontSet* operator->() const {
            return value;
        }
    };
    FcFontSetWrap fs(FcFontList(nullptr, pattern, os));
    FcObjectSetDestroy(os);
    if (fs && fs->nfont > 0) {
        auto f = fs->fonts[0];
        FcChar8* str;
        FcPatternGetString(f, FC_FILE, 0, &str);
        AString result = (const char*)str;
        FcPatternDestroy(pattern);
        return result;
    } else {
        return {};
    }
}