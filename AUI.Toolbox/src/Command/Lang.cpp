//
// Created by alex2772 on 11/6/20.
//

#include <AUI/IO/APath.h>
#include "Lang.h"


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

void Lang::run(Toolbox& t) {
    switch (t.args.size()) {
        case 0:
            throw IllegalArgumentsException("lang requires project path");

        case 1:
            throw IllegalArgumentsException("lang requires subcommand (generate|update)");

    }

    if (t.args[1] == "generate") {
        if (t.args.size() != 2) {
            throw IllegalArgumentsException("lang generate requires a language argument");
        }
        auto path = APath(t.args[0]).file("assets/lang/{}.lang"_as.format(t.args[2]));
        std::cout << "target file: " << path << std::endl;

    } else if (t.args[1] == "update") {

    } else {
        throw IllegalArgumentsException("lang accepts generate or update subcommand");
    }
}
