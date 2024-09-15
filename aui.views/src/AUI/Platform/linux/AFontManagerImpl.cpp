/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/AFontManager.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fontconfig/fontconfig.h>
#include <AUI/IO/APath.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Util/ATokenizer.h>
#include "AUI/Font/FreeType.h"
#include "AUI/Util/ARaiiHelper.h"

static constexpr auto LOG_TAG = "FontManager";

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
                    auto url = AUrl::file(getPathToFont(fontName));
                    result = loadFont(std::move(url));
                    ALogger::info(LOG_TAG) << "Using gtk theme font: " << fontName << " (" << url.full() << ")";
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
                    auto url = AUrl::file(getPathToFont(d));
                    ALogger::info(LOG_TAG) << "Using system default font: " << d << "(" << url.full() << ")";
                    result = loadFont(std::move(url));
                    break;
                } catch (...) {}
            }
        }

        return result;
    }();

    if (mDefaultFont == nullptr) {
        // fallback to internal
        mDefaultFont = loadFont(":uni/font/Roboto.ttf");
        ALogger::info(LOG_TAG) << "Using fallback internal font";
    }
}

namespace {
    namespace pattern {
        template<typename T>
        T get(FcPattern* pattern, const char *object, int n = 0) = delete;

        template<>
        AString get(FcPattern* pattern, const char *object, int n) {
            FcChar8 *str;
            FcPatternGetString(pattern, object, n, &str);
            return reinterpret_cast<const char *>(str);
        }

        template<>
        int get(FcPattern* pattern, const char *object, int n) {
            int i;
            FcPatternGetInteger(pattern, object, n, &i);
            return i;
        }
    }
    struct Pattern: aui::noncopyable {
    public:
        Pattern(FcPattern *pattern) : pattern(pattern) {}

        ~Pattern() {
            FcPatternDestroy(pattern);
        }

        operator FcPattern*() const noexcept {
            return pattern;
        }

        template<typename T>
        T get(const char *object, int n = 0) {
            return pattern::get<T>(pattern, object, n);
        }

    private:
        FcPattern* pattern;
    };
}


AString AFontManager::getPathToFont(const AString& family) {
    if (APath(family.toStdString()).isRegularFileExists())
        return family;

    // reference: qt ./qtbase/src/gui/text/unix/qfontconfigdatabase.cpp

    Pattern pattern = FcPatternCreate();
    auto tmp = family.toStdString();
    //FcDefaultSubstitute(pattern);
    auto os = FcObjectSetBuild (FC_FILE,
                                FC_WIDTH,
                                FC_STYLE,
                                nullptr);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(tmp.c_str()));
    FcPatternAddString(pattern, FC_STYLE, reinterpret_cast<const FcChar8*>("Regular"));
    FcPatternAddInteger(pattern, FC_WIDTH, FC_WIDTH_NORMAL);

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
        return pattern::get<AString>(fs->fonts[0], FC_FILE);
    } else {
        return {};
    }
}