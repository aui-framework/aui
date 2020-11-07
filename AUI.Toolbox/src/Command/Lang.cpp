//
// Created by alex2772 on 11/6/20.
//

#include <AUI/IO/APath.h>
#include "Lang.h"
#include <AUI/Traits/strings.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/IO/FileOutputStream.h>
#include <AUI/Util/ATokenizer.h>

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
    for (auto& i : srcDir.listDir(LF_RECURSIVE | LF_DEFAULT_FLAGS)) {
        if (!i.endsWith(".cpp") && !i.endsWith(".h") && i.endsWith(".hpp")) {
            continue;
        }

        try {
            ATokenizer t(_new<FileInputStream>(i));
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
    auto fos = _new<FileOutputStream>(path);
    *fos << "# AUI lang file\n";

    for (auto& i : data) {
        *fos << i.first.replaceAll("=", "\\=") << "=" << i.second.replaceAll("=", "\\=") << "\n";
    }
}

void Lang::run(Toolbox& t) {
    switch (t.args.size()) {
        case 0:
            throw IllegalArgumentsException("lang requires project path");

        case 1:
            throw IllegalArgumentsException("lang requires subcommand (generate|update)");

    }

    if (t.args[1] == "generate") {
        if (t.args.size() != 3) {
            throw IllegalArgumentsException("lang generate requires a language argument");
        }
        auto langFile = APath(t.args[0]).absolute().file("assets/lang/{}.lang"_as.format(t.args[2]));
        if (langFile.isRegularFileExists()) {
            std::cout << "target file " << langFile << " already exists - use lang update to update this file" << std::endl;
            //return;
        }
        langFile.parent().makeDirs();
        auto srcDir = APath(t.args[0]).absolute().file("src");
        AMap<AString, AString> dst;
        scanSrcDir(srcDir, dst);
        saveLangFile(langFile, dst);
        std::cout << "created file " << langFile << std::endl
                  << "don't forget to add your new language files to your version control." << std::endl;
    } else if (t.args[1] == "update") {

    } else {
        throw IllegalArgumentsException("lang accepts generate or update subcommand");
    }
}
