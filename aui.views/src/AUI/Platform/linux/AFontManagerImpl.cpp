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

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <AUI/Platform/AFontManager.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fontconfig/fontconfig.h>
#include <AUI/IO/APath.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include "AUI/Render/FreeType.h"

extern Display* gDisplay;
void ensureXLibInitialized();


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
                    _new<FileInputStream>("/home/{}/.config/xsettingsd/xsettingsd.conf"_format(getenv("USER"))));

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