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

//
// Created by alex2772 on 11/6/20.
//

#include <AUI/IO/APath.h>
#include "Lang.h"
#include <AUI/Traits/strings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/i18n/ALanguageCode.h>
#include <AUI/i18n/AI18n.h>

AString Lang::getName() {
    return "lang";
}

AString Lang::getSignature() {
    return "<project_dir> [generate <lang in ISO 639-1>|update]";
}

AString Lang::getDescription() {
    return "generates a new .*lang file or updates all existing *.lang files in <project_dir>/assets/lang folder. This "
           "tool analyses source files in <project_dir>/src and stores '_i18n' string literals into file.";
}

void scanSrcDir(const APath& srcDir, AMap<AString, AString>& dst) {
    for (auto& i : srcDir.listDir(AFileListFlags::RECURSIVE | AFileListFlags::DEFAULT_FLAGS)) {
        if (!i.endsWith(".cpp") && !i.endsWith(".h") && i.endsWith(".hpp")) {
            continue;
        }

        try {
            ATokenizer t(_new<AFileInputStream>(i));
            for (;;) {
                t.readStringUntilUnescaped('"');
                AString stringLiteral = t.readStringUntilUnescaped('"');
                if (t.readString(5) == "_i18n") {
                    if (!dst.contains(stringLiteral)) {
                        dst[stringLiteral] = stringLiteral;
                    }
                }
            }
        } catch (...) {

        }
    }
}

void saveLangFile(const APath& path, const AMap<AString, AString>& data) {
    auto fos = _new<AFileOutputStream>(path);
    fos << "# AUI lang file\n";

    for (auto& i : data) {
        *fos << i.first.replacedAll("=", "\\=") << "=" << i.second.replacedAll("=", "\\=") << "\n";
    }
}

void Lang::run(Toolbox& t) {
    switch (t.args.size()) {
        case 0:
            throw IllegalArgumentsException("lang requires project path");

        case 1:
            throw IllegalArgumentsException("lang requires subcommand (generate|update)");

    }
    auto srcDir = APath(t.args[0]).absolute().file("src");

    if (t.args[1] == "generate") {
        if (t.args.size() != 3) {
            throw IllegalArgumentsException("lang generate requires a language argument");
        }
        APath langFile;
        try {
            langFile = APath(t.args[0]).absolute().file(
                    "assets/lang/{}.lang"_format(ALanguageCode(t.args[2]).toString()));
        } catch (const AException& e) {
            std::cout << e.getMessage() << std::endl
                      << "note: AUI follows the ISO 639-1 format of language codes (en-US, ru-RU, ru-UA etc...)"
                      << std::endl;
            return;
        }
        if (langFile.isRegularFileExists()) {
            std::cout << "target file " << langFile << " already exists - use 'lang update' to update this file" << std::endl;
            return;
        }
        langFile.parent().makeDirs();
        AMap<AString, AString> dst;
        scanSrcDir(srcDir, dst);
        saveLangFile(langFile, dst);
        std::cout << "created file " << langFile << std::endl
                  << "don't forget to add your new language files to your version control." << std::endl;
    } else if (t.args[1] == "update") {
        AMap<AString, AString> dst;
        scanSrcDir(srcDir, dst);
        auto langDir = APath(t.args[0]).absolute().file("assets/lang/");
        for (auto& l : langDir.listDir()) {
            auto filename = l.filename();
            auto dotPos = filename.AString::find('.');
            if (dotPos == AString::NPOS)
                continue;

            AString langName;

            try {
                langName = ALanguageCode(filename.substr(0, dotPos)).toString();
            } catch (...) {
                continue;
            }

            AMap<AString, AString> dstForThisLang = dst;
            AI18n::loadFromStreamInto(_new<AFileInputStream>(l), dstForThisLang);
            saveLangFile(l, dstForThisLang);
            std::cout << "updated: " << l << std::endl;
        }
    } else {
        throw IllegalArgumentsException("lang accepts generate or update subcommand");
    }
}
