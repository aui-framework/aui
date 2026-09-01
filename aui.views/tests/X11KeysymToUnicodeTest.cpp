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

#include <gtest/gtest.h>
#include <clocale>
#include <string>

#if AUI_PLATFORM_LINUX
#include <langinfo.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/linux/x11/keysym_to_unicode.h>
TEST(X11KeysymToUnicodeTest, AsciiAndLatin1) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0020), 0x0020); // space
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0041), 'A');
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0061), 'a');
    EXPECT_EQ(aui::x11::keysymToUnicode(0x00e9), 0x00e9); // eacute (Latin-1)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x00a9), 0x00a9); // copyright
}

TEST(X11KeysymToUnicodeTest, DirectUnicode) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x010000ff), 0);      // below lower bound (0x01000100) -> 0
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01000100), 0x0100); // lower bound U+0100
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01000452), 0x0452); // direct Unicode U+0452
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01003002), 0x3002); // direct Unicode U+3002
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0110ffff), 0x10ffff); // upper bound U+10FFFF
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01110000), 0);      // above upper bound -> 0
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0100d800), 0);      // surrogate start -> 0
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0100dfff), 0);      // surrogate end -> 0
}

TEST(X11KeysymToUnicodeTest, Cyrillic) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06a1), 0x0452); // Serbian_dje (U+0452)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06a2), 0x0453); // Macedonia_gje (U+0453)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06a3), 0x0451); // Cyrillic_io (U+0451)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06b1), 0x0402); // Serbian_DJE (U+0402)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06b3), 0x0401); // Cyrillic_IO (U+0401)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06c1), 0x0430); // Cyrillic_a (U+0430)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06e1), 0x0410); // Cyrillic_A (U+0410)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06b0), 0x2116); // numero (U+2116)
}

TEST(X11KeysymToUnicodeTest, Katakana) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04a1), 0x3002); // kana_fullstop (U+3002)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04a2), 0x300c); // kana_openingbracket (U+300C)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04a3), 0x300d); // kana_closingbracket (U+300D)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04a4), 0x3001); // kana_comma (U+3001)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04b1), 0x30a2); // kana_A (U+30A2)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04df), 0x309c); // semivoicedsound (U+309C)
}

TEST(X11KeysymToUnicodeTest, Hebrew) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0cdf), 0x2017); // hebrew_doublelowline (U+2017)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0ce0), 0x05d0); // hebrew_aleph (U+05D0)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0ce1), 0x05d1); // hebrew_bet (U+05D1)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0cfa), 0x05ea); // hebrew_taw (U+05EA)
}

TEST(X11KeysymToUnicodeTest, Arabic) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x05ac), 0x060c); // Arabic_comma (U+060C)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x05c1), 0x0621); // Arabic_hamza (U+0621)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x05c7), 0x0627); // Arabic_alef (U+0627)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x05f2), 0x0652); // Arabic_sukun (U+0652)
}

TEST(X11KeysymToUnicodeTest, HangulKorean) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0ea1), 0x3131); // Hangul_Kiyeog (U+3131)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0ed4), 0x11a8); // Hangul_J_Kiyeog (U+11A8)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0ef6), 0x318d); // Hangul_AraeA (U+318D)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x0eff), 0x20a9); // Korean_Won (U+20A9)
}

TEST(X11KeysymToUnicodeTest, Greek) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x07a1), 0x0386); // Greek_ALPHAaccent (U+0386)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x07c1), 0x0391); // Greek_ALPHA (U+0391)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x07e1), 0x03b1); // Greek_alpha (U+03B1)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x07f9), 0x03c9); // Greek_omega (U+03C9)
}

TEST(X11KeysymToUnicodeTest, Latin234AndOthers) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01a1), 0x0104); // Aogonek (U+0104)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01b1), 0x0105); // aogonek (U+0105)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x13bc), 0x0152); // OE ligature (U+0152)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x13bd), 0x0153); // oe ligature (U+0153)
    EXPECT_EQ(aui::x11::keysymToUnicode(0x20ac), 0x20ac); // EuroSign (U+20AC)
}

TEST(X11KeysymToUnicodeTest, KeypadAndControlKeys) {
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff80), 0x20);   // KP_Space
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffb0), '0');    // KP_0
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffb9), '9');    // KP_9
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffaa), '*');    // KP_Multiply
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffab), '+');    // KP_Add
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff0d), 0x0d);   // Return
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff1b), 0x1b);   // Escape
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffff), 0x7f);   // Delete
}

TEST(X11KeysymToUnicodeTest, LocaleFallbackUnderCLocale) {
    const char* cur = std::setlocale(LC_ALL, nullptr);
    const std::string savedLocale = cur ? cur : "C";
    struct LocaleRestorer {
        std::string saved;
        ~LocaleRestorer() {
            std::setlocale(LC_ALL, saved.c_str());
        }
    } restorer{savedLocale};

    // Simulate environment selecting C locale
    std::setlocale(LC_ALL, "C");
    aui::detail::initUtf8Locale();

    // initUtf8Locale must leave a UTF-8 codeset selected and preserve LC_NUMERIC=C
#if AUI_PLATFORM_LINUX
    const char* codeset = nl_langinfo(CODESET);
    ASSERT_NE(codeset, nullptr);
    EXPECT_STRCASEEQ(codeset, "UTF-8");
#endif
    const char* numLoc = std::setlocale(LC_NUMERIC, nullptr);
    ASSERT_NE(numLoc, nullptr);
    EXPECT_STREQ(numLoc, "C");
    // Non-ASCII input through keysymToUnicode works accurately regardless of initial locale
    EXPECT_EQ(aui::x11::keysymToUnicode(0x06a1), 0x0452); // Cyrillic
    EXPECT_EQ(aui::x11::keysymToUnicode(0x04a1), 0x3002); // Katakana
    EXPECT_EQ(aui::x11::keysymToUnicode(0x00e9), 0x00e9); // Latin-1 eacute
    EXPECT_EQ(aui::x11::keysymToUnicode(0x01000452), 0x0452); // Direct Unicode
}

TEST(X11KeysymToUnicodeTest, ControlCharacterFiltering) {
    // Control keysyms convert to ASCII control codes (< 32 or == 127) and are filtered from onCharEntered
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff08), 0x08); // BackSpace (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff09), 0x09); // Tab (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff0a), 0x0a); // Linefeed (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff0b), 0x0b); // Clear (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff0d), 0x0d); // Return (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xff1b), 0x1b); // Escape (< 32)
    EXPECT_EQ(aui::x11::keysymToUnicode(0xffff), 0x7f); // Delete (== 127)
}
#endif
