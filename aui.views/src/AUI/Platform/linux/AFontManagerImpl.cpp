/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/AFontManager.h>

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

    // Pre-warm fallback face to avoid UI-thread stall on first missing-glyph render.
    initFallback();
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    // Use fontconfig to find a CJK-capable sans-serif font.
    // Ensure fontconfig is initialized before using default config (nullptr).
    if (!FcInit()) {
        ALogger::warn(LOG_TAG) << "FcInit() failed; CJK fallback unavailable";
        return {};
    }
    // Use a charset-based query with representative codepoints from each CJK
    // script (Han, Hiragana, Hangul) so that Kana-only and Hangul-only fonts
    // can enter the candidate list, not just Han-capable ones.
    static constexpr FcChar32 kProbeCodepoints[] = { 0x4E2D /*中*/, 0x3042 /*あ*/, 0xAC00 /*가*/ };
    FcPattern* pattern = FcPatternCreate();
    FcCharSet* charset = FcCharSetCreate();
    if (!pattern || !charset) {
        // Allocation failure (memory exhaustion): never call fontconfig APIs
        // with null handles (FcPatternAddCharSet would crash). Discovery is
        // best-effort; log and leave the fallback pool empty.
        if (pattern) FcPatternDestroy(pattern);
        if (charset) FcCharSetDestroy(charset);
        ALogger::warn(LOG_TAG) << "fontconfig allocation failed; CJK fallback unavailable";
        return {};
    }
    for (auto probe : kProbeCodepoints) {
        FcCharSetAddChar(charset, probe);
    }
    FcPatternAddCharSet(pattern, FC_CHARSET, charset);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>("sans"));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcCharSetDestroy(charset);

    // Use FcFontSort to obtain an ordered set of CJK-capable candidates.
    // FcFontMatch would only return a single best match, which cannot populate
    // the deferred-candidate pool for lazy loading on codepoint miss.
    // The list is capped: loaded fallback faces are never released, so an
    // unbounded candidate pool would keep opening font files for the whole
    // process lifetime on codepoint misses (e.g. emoji, rare symbols).
    constexpr int kMaxFallbackCandidates = 8;
    AVector<AFontManager::FallbackCandidate> candidates;
    FcResult result;
    FcFontSet* fontSet = FcFontSort(nullptr, pattern, FcTrue, nullptr, &result);
    if (fontSet) {
        for (int i = 0; i < fontSet->nfont && candidates.size() < kMaxFallbackCandidates; ++i) {
            FcPattern* font = fontSet->fonts[i];
            // Verify the font covers at least one of the CJK scripts.
            FcCharSet* fontCharset = nullptr;
            if (FcPatternGetCharSet(font, FC_CHARSET, 0, &fontCharset) == FcResultMatch) {
                bool coversAny = false;
                for (auto probe : kProbeCodepoints) {
                    if (FcCharSetHasChar(fontCharset, probe)) {
                        coversAny = true;
                        break;
                    }
                }
                if (!coversAny) {
                    continue;   // Skip fonts that cover none of the CJK scripts.
                }
            }
            FcChar8* path = nullptr;
            int faceIndex = 0;
            if (FcPatternGetString(font, FC_FILE, 0, &path) == FcResultMatch) {
                FcPatternGetInteger(font, FC_INDEX, 0, &faceIndex);
                candidates.push_back({ AString(reinterpret_cast<const char*>(path)), faceIndex });
            }
        }
        FcFontSetDestroy(fontSet);
    }
    FcPatternDestroy(pattern);
    if (candidates.empty()) {
        ALogger::warn(LOG_TAG) << "No CJK fallback font found via fontconfig";
    }
    return candidates;
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