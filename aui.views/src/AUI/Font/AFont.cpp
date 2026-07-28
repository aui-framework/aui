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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include "AUI/Font/FreeType.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Logging/ALogger.h"
#include <fstream>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"

#if AUI_PLATFORM_LINUX
#include <fontconfig/fontconfig.h>
#endif


AFont::AFont(AFontManager* fm, const AString& path) :
        ft(fm->mFreeType) {
    if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &mFace)) {
        throw AException("Could not load font: " + path);
    }
}

AFont::AFont(AFontManager* fm, const AUrl& url) :
        ft(fm->mFreeType) {
    if (url.schema() == "file") {
        if (FT_New_Face(fm->mFreeType->getFt(), url.path().toStdString().c_str(), 0, &mFace)) {
            throw AException("Could not load font: " + url.full());
        }
        return;
    }
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(),
                           0, &mFace)) {
        throw AException("Could not load font: " + url.full());
    }
}

AFont::~AFont() {
    if (mFallbackFace) {
        FT_Done_Face(mFallbackFace);
    }
    if (mFace) {
        FT_Done_Face(mFace);
    }
}

AString AFont::getFontFamilyName() const {
    FT_SfntName name;
    FT_Get_Sfnt_Name(mFace, 0, &name);
    return std::string(name.string, name.string + name.string_len);
}

AFontFamily::Weight AFont::getFontWeight() const {
    return AFontFamily::NORMAL;
}

bool AFont::isItalic() const {
    return mFace->style_flags & FT_STYLE_FLAG_ITALIC;
}

bool AFont::hasGlyph(char32_t codepoint) const {
    return FT_Get_Char_Index(mFace, codepoint) != 0;
}

void AFont::ensureFallbackFace() {
    if (mFallbackFace) {
        return; // already loaded
    }
    if (mFallbackAttempted) {
        // Already tried and failed; don't retry every glyph
        return;
    }
    mFallbackAttempted = true;

#if AUI_PLATFORM_LINUX
    // Use fontconfig to find a CJK-capable sans-serif font.
    // Ensure fontconfig is initialized before using default config (nullptr).
    if (!FcInit()) {
        ALogger::warn("Font") << "FcInit() failed; CJK fallback unavailable";
        return;
    }
    // Use a charset-based query with a common CJK character to ensure the font actually has CJK glyphs.
    FcPattern* pattern = FcPatternCreate();
    FcCharSet* charset = FcCharSetCreate();
    FcCharSetAddChar(charset, 0x4E2D); // U+4E2D = '中' (common CJK character)
    FcPatternAddCharSet(pattern, FC_CHARSET, charset);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>("sans"));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcCharSetDestroy(charset);

    FcResult result;
    FcPattern* match = FcFontMatch(nullptr, pattern, &result);
    if (match) {
        if (result == FcResultMatch) {
            FcChar8* path = nullptr;
            if (FcPatternGetString(match, FC_FILE, 0, &path) == FcResultMatch) {
                mFallbackFontPath = reinterpret_cast<const char*>(path);

                // Try to load the fallback font face.
                // TTC collections may need face_index > 0; try index 0 first (usually Regular).
                if (FT_New_Face(ft->getFt(), mFallbackFontPath.toStdString().c_str(), 0, &mFallbackFace)) {
                    ALogger::warn("Font") << "Failed to load fallback font: " << mFallbackFontPath;
                    mFallbackFace = nullptr;
                    mFallbackFontPath.clear();
                } else {
                    ALogger::info("Font") << "Loaded CJK fallback font: " << mFallbackFontPath;
                }
            }
        }
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pattern);
#elif AUI_PLATFORM_WIN
    // Try known CJK font files from the Windows Fonts directory.
    // Order: most-comprehensive first (Microsoft YaHei covers CJK + Kana,
    // Malgun Gothic covers CJK + Hangul, then region-specific fallbacks).
    const char* cjkFonts[] = {
        "msyh.ttc",       // Microsoft YaHei (Simplified Chinese, includes CJK + Kana)
        "malgun.ttf",     // Malgun Gothic (Korean, includes Hangul + CJK)
        "simsun.ttc",     // SimSun (Chinese Traditional)
        "msgothic.ttc",   // MS Gothic (Japanese)
        "yugothic.ttf",   // Yu Gothic (Japanese)
        "meiryo.ttc",     // Meiryo (Japanese)
    };
    static const AString fontsDir = [] {
        char buf[MAX_PATH];
        UINT len = GetWindowsDirectoryA(buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            return AString(buf) + "\\Fonts\\";
        }
        ALogger::warn("Font") << "GetWindowsDirectoryA() failed, falling back to C:\\Windows\\Fonts\\";
        return AString("C:\\Windows\\Fonts\\");
    }();
    for (auto& f : cjkFonts) {
        AString fontPath = fontsDir + f;
        if (FT_New_Face(ft->getFt(), fontPath.toStdString().c_str(), 0, &mFallbackFace) == 0) {
            mFallbackFontPath = fontPath;
            ALogger::info("Font") << "Loaded CJK fallback font: " << mFallbackFontPath;
            break;
        }
    }
    if (!mFallbackFace) {
        ALogger::warn("Font") << "No CJK fallback font found on Windows";
    }
#elif AUI_PLATFORM_MACOS
    // macOS system CJK fonts (stable paths since OS X 10.11).
    const char* cjkFontsMac[] = {
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
        "/System/Library/Fonts/Hiragino Sans.ttc",
        "/System/Library/Fonts/Supplemental/AppleSDGothicNeo.ttc",
    };
    for (auto& f : cjkFontsMac) {
        AString fontPath(f);
        if (FT_New_Face(ft->getFt(), fontPath.toStdString().c_str(), 0, &mFallbackFace) == 0) {
            mFallbackFontPath = fontPath;
            ALogger::info("Font") << "Loaded CJK fallback font: " << mFallbackFontPath;
            break;
        }
    }
    if (!mFallbackFace) {
        ALogger::warn("Font") << "No CJK fallback font found on macOS";
    }
#elif AUI_PLATFORM_ANDROID
    // Android system CJK fonts (varies by API level).
    const char* cjkFontsAndroid[] = {
        "/system/fonts/NotoSansCJK-Regular.ttc",  // Android 5-9
        "/system/fonts/NotoSansSC-Regular.otf",    // Android 10+ (Chinese)
        "/system/fonts/NotoSansKR-Regular.otf",    // Android 10+ (Korean)
        "/system/fonts/NotoSansJP-Regular.otf",    // Android 10+ (Japanese)
    };
    for (auto& f : cjkFontsAndroid) {
        AString fontPath(f);
        if (FT_New_Face(ft->getFt(), fontPath.toStdString().c_str(), 0, &mFallbackFace) == 0) {
            mFallbackFontPath = fontPath;
            ALogger::info("Font") << "Loaded CJK fallback font: " << mFallbackFontPath;
            break;
        }
    }
    if (!mFallbackFace) {
        ALogger::warn("Font") << "No CJK fallback font found on Android";
    }
#elif AUI_PLATFORM_IOS
    // iOS: try system font paths first (some iOS versions allow reading them),
    // then fall back to a bundled resource if available.
    const char* cjkFontsIos[] = {
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
    };
    {
        bool loaded = false;
        for (auto& f : cjkFontsIos) {
            AString fontPath(f);
            if (FT_New_Face(ft->getFt(), fontPath.toStdString().c_str(), 0, &mFallbackFace) == 0) {
                mFallbackFontPath = fontPath;
                ALogger::info("Font") << "Loaded CJK fallback font: " << mFallbackFontPath;
                loaded = true;
                break;
            }
        }
        if (!loaded) {
            // Try bundled resource
            try {
                mFallbackFontDataBuffer = AByteBuffer::fromStream(AUrl(":uni/font/NotoSansCJK-Fallback.ttf").open());
                if (FT_New_Memory_Face(ft->getFt(),
                        (const FT_Byte*) mFallbackFontDataBuffer.data(),
                        mFallbackFontDataBuffer.getSize(), 0, &mFallbackFace) == 0) {
                    mFallbackFontPath = AString(":uni/font/NotoSansCJK-Fallback.ttf");
                    ALogger::info("Font") << "Loaded CJK fallback font from bundle";
                    loaded = true;
                }
            } catch (...) {
                // bundled font not present
            }
        }
        if (!loaded) {
            ALogger::warn("Font") << "No CJK fallback font found on iOS";
        }
    }
#else
    // Unknown platform; mark fallback as unavailable.
    ALogger::warn("Font") << "CJK font fallback not available on this platform";
#endif
}

glm::vec2 AFont::getKerning(wchar_t left, wchar_t right) {
    FT_Vector vec2;
    FT_Get_Kerning(mFace, left, right, FT_KERNING_DEFAULT, &vec2);

    return {vec2.x >> 6, vec2.y >> 6};
}

AFont::Character AFont::renderGlyph(const FontEntry& fs, AChar glyph) {
    int size = fs.first.size;
    FontRendering fr = fs.first.fr;

    // Determine which face to use: try primary, fall back to CJK font if glyph missing.
    FT_Face face = nullptr;
    if (hasGlyph(glyph.codepoint())) {
        face = mFace;
    } else {
        ensureFallbackFace();
        if (mFallbackFace) {
            face = mFallbackFace;
        }
    }
    if (!face) {
        // No font has this glyph; return empty character.
        return Character{};
    }

    FT_Set_Pixel_Sizes(face, 0, size);

    FT_Int32 flags = FT_LOAD_RENDER;

    if (fr == FontRendering::SUBPIXEL)
        flags |= FT_LOAD_TARGET_LCD;
    if (fr == FontRendering::NEAREST)
        flags |= FT_LOAD_TARGET_MONO;

    FT_Error e = FT_Load_Char(face, glyph.codepoint(), flags);
    if (e) {
        // Neither face can render this glyph.
        ALogger::debug("Font") << "FT_Load_Char failed for U+"
                               << std::hex << std::uint32_t(glyph.codepoint()) << " (error " << std::dec << e << ")";
        return Character{};
    }
    FT_GlyphSlot g = face->glyph;
    if (g->bitmap.width && g->bitmap.rows) {
        const float div = 1.f / 64.f;
        int width = g->bitmap.width;

        if (fr == FontRendering::SUBPIXEL)
            width /= 3;

        int height = g->bitmap.rows;

        AByteBuffer data;

        if (fr == FontRendering::NEAREST) {
            // when nearest, freetype renders glyphs into the 1bit-depth image but OpenGL required at least8bit-depth,
            // so we will convert it here
            data.resize(g->bitmap.rows * g->bitmap.width);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                for (unsigned c = 0; c < g->bitmap.width; ++c) {
                    data.at<std::uint8_t>(c + r * g->bitmap.width) = (bufPtr[c / 8] & (0b10000000 >> (c % 8))) ? 255
                                                                                                               : 0;
                }
            }
        } else {
            data.reserve(g->bitmap.rows * g->bitmap.pitch);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                data.write(reinterpret_cast<const char*>(bufPtr), g->bitmap.width);
            }
        }

        int imageFormat = APixelFormat::BYTE;
        if (fr == FontRendering::SUBPIXEL)
            imageFormat |= APixelFormat::RGB;
        else
            imageFormat |= APixelFormat::R;

        return Character{
            .image = _new<AImage>(data, glm::uvec2(width, height), imageFormat),
            .size = { div * g->metrics.width, div * g->metrics.height },
            .horizontal = {
              .bearing = { g->bitmap_left, g->bitmap_top },
              .advance = div * g->metrics.horiAdvance,
            },
            .vertical = {
              .bearing = { div * g->metrics.vertBearingX, div * g->metrics.vertBearingY },
              .advance = div * g->metrics.vertAdvance,
            },
        };
    }
    return Character{};
}

AFont::Character& AFont::getCharacter(const FontEntry& charset, AChar glyph) {
    auto& chars = charset.second.characters;
    if (chars.size() > glyph && chars[glyph.codepoint()]) {
        return *chars[glyph.codepoint()];
    } else {
        if (chars.size() <= glyph) {
            chars.resize(glyph + 1, std::nullopt);
        }
        chars[glyph.codepoint()] = std::move(renderGlyph(charset, glyph));

        return *chars[glyph.codepoint()];
    }
}

int AFont::length(const FontEntry& charset, AStringView text) {
    return length(charset, text.utf8().begin(), text.utf8().end());
}

int AFont::length(const FontEntry& charset, std::u32string_view text) {
    return length(charset, text.begin(), text.end());
}

bool AFont::isHasKerning() {
    return FT_HAS_KERNING(mFace);
}

int AFont::getAscenderHeight(unsigned size) const {
    return int(mFace->ascender) * size / mFace->height;
}


int AFont::getDescenderHeight(unsigned size) const {
    return -int(mFace->descender) * size / mFace->height;
}
