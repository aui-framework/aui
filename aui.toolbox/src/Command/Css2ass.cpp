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

//
// Created by alex2 on 07.01.2021.
//


#include "Css2ass.h"

#include <AUI/IO/APath.h>
#include "Css2ass.h"
#include <AUI/Traits/strings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Util/ARandom.h>

AString Css2ass::getName() {
    return "css2ass";
}

AString Css2ass::getSignature() {
    return "<path to source css> <path to application src/ folder> [-f]";
}

AString Css2ass::getDescription() {
    return "converts CSS (cascading style sheets) to the src/Style.cpp file containing appropriate ASS (aui style sheets) "
           "definitions. aui.toolbox will use src/ folder to find files to #include. Please note that this command will "
           "not generate 100% equal ASS analogue to CSS because of differences between ASS and CSS.\n"
           "\t-f\tignore already existent destination file (warning! overrides contents)";
}

void Css2ass::run(Toolbox& t) {
    if (t.args.size() >= 2) {
        APath sourceCssPath = t.args[0];
        APath srcPath = t.args[1];
        APath destinationAssPath = srcPath.file("Style.cpp");

        if (!sourceCssPath.isRegularFileExists()) {
            throw AException("source file " + sourceCssPath + " does not exist");
        }

        if (destinationAssPath.isRegularFileExists() && !t.args.contains("-f")) {
            throw AException("destination file " + sourceCssPath + " already exists. Please specify another "
                             "destination file or specify the f flag (warning! f flag overrides contents)");
        }

        ATokenizer t(_new<AFileInputStream>(sourceCssPath));

        // for #include <...> list
        AVector<AString> includes;

        // code
        AVector<AString> code;

        // processed includes to avoid duplicates like AButton, ALabel, etc...
        ASet<AString> processedTypes;

        // read css
        try {
            char c;
            // declaration loop
            for (;;) {
                c = t.readChar();
                if (!isspace(c)) {
                    t.reverseByte();

                    code << "    {";

                    // selector loop
                    AString currentSubSelectorAss;
                    AStringVector subSelectorAss;
                    enum class Relationship {
                        UNSET,
                        SET,
                        SET_PARENT,
                    } relation = Relationship::UNSET;
                    for (bool running = true; running; ) {
                        AString subSelectorCss = t.readString({'.', '#', ':', '_'});
                        if (!subSelectorCss.empty()) {
                            if (relation == Relationship::SET_PARENT) {
                                // looks like its really relationship specifier, not just some space symbol.
                                currentSubSelectorAss += " >> ";
                            }
                            relation = Relationship::UNSET;

                            size_t colonIndex = subSelectorCss.find(':');

                            AString subClass;
                            if (colonIndex != AString::NPOS) {
                                subClass = subSelectorCss.substr(colonIndex + 1);
                                subSelectorCss = subSelectorCss.substr(0, colonIndex);
                            }

                            // selector routine
                            if (subSelectorCss.startsWith(".") || subSelectorCss.startsWith("#")) {
                                // css class or id
                                currentSubSelectorAss += "class_of(\"";
                                currentSubSelectorAss += subSelectorCss.substr(1);
                                currentSubSelectorAss += "\")";
                            } else {
                                // some type
                                currentSubSelectorAss += "any<";
                                currentSubSelectorAss += subSelectorCss;
                                currentSubSelectorAss += ">";
                                if (!processedTypes.contains(subSelectorCss)) {
                                    processedTypes << subSelectorCss;
                                    bool found = false;
                                    // try to find type's header file in order to include it
                                    for (auto& targetFile : srcPath.listDir(AFileListFlags::REGULAR_FILES | AFileListFlags::RECURSIVE)) {
                                        // only for C++ headers
                                        if (targetFile.endsWith(".h") || targetFile.endsWith(".hpp")) {
                                            if (targetFile.filenameWithoutExtension() == subSelectorCss) {
                                                includes << "\"" + targetFile.relativelyTo(srcPath) + "\"";
                                                found = true;
                                            }
                                        }
                                    }
                                    if (!found && subSelectorCss.startsWith('A')) {
                                        // if file not found and subSelectorCss starts with 'A' means we can include the
                                        // AUI's header because subSelectorCss starting with 'A' most likely belongs to
                                        // AUI
                                        includes << "<AUI/View/" + subSelectorCss + ".h>";

                                        // we should notify user about it
                                        std::cout << "assuming   " << subSelectorCss << " belongs to AUI" << std::endl;
                                    } else {
                                        // notify user that we cannot find file where this type defined in.
                                        std::cout << "unresolved " << subSelectorCss << std::endl;
                                    }
                                }
                                currentSubSelectorAss += "()";
                            }
                        } else {
                            // determine why can't we read next subselector
                            switch (t.readChar()) {
                                case '{':
                                    // done for selectors. rule body begins
                                    if (!currentSubSelectorAss.empty()) {
                                        // push the current one
                                        subSelectorAss << currentSubSelectorAss;
                                    }
                                    running = false;
                                    break;

                                case ',':
                                    // another subselector. push the current one
                                    subSelectorAss << currentSubSelectorAss;
                                    currentSubSelectorAss.clear();
                                    relation = Relationship::SET;
                                    break;

                                case ' ':
                                    // parent relationship or just some space symbol. raise parent relationship flag
                                    if (relation == Relationship::UNSET) {
                                        relation = Relationship::SET_PARENT;
                                    }
                                    break;

                                case '>':
                                    // direct parent relationship. Just put ">" into the ASS subselector and continue
                                    // parsing
                                    currentSubSelectorAss += " > ";
                                    relation = Relationship::SET;
                                    break;
                            }
                        }
                    }
                    code << "        " + (subSelectorAss.size() == 1 ? subSelectorAss.first() + "," : "{ " + subSelectorAss.join(", ") + " },");

                    // now we in the rule body
                    // key: arguments;
                    for (;;) {
                        AString line = AString(t.readStringUntilUnescaped({';', '\n'})).trim();

                        auto colonIndex = line.find(':');
                        if (colonIndex != AString::NPOS) {
                            AString property = line.substr(0, colonIndex);
                            AString value = line.substr(colonIndex + 1).trim();

                            try {
                                processRule(code, property, value);
                            } catch (const AException& e) {
                                std::cerr << sourceCssPath.filename() << ':' << t.getRow() << ' ' << e.getMessage()
                                          << std::endl;
                            }
                        }
                        // read excess semicolon or new line
                        c = t.readChar();
                        if (c == '}' || line.contains('}'))
                            break;
                    }

                    code << "    },\n";
                }
            }
        } catch (...) {

        }

        auto fos = _new<AFileOutputStream>(destinationAssPath);
        fos << "#include <AUI/ASS/ASS.h>\n";
        includes.sort();
        for (auto& line : includes) {
            fos << "#include " << line << '\n';
        }
        fos << "\nusing namespace ass;\n";
        AString randomToken = ARandom().nextBytes(4).toHexString();
        fos << "\n"
               "struct Style" << randomToken << " {\n"
               "    Style" << randomToken << "() {\n"
               "        AStylesheet::global().addRules({\n";

        for (auto& line : code) {
            fos << "        " << line << '\n';
        }


        fos << "        });\n"
               "    }\n"
               "} style" << randomToken << ";\n";


    } else {
        throw IllegalArgumentsException("css2ass requires two arguments");
    }
}

struct PropertyWrapper {
private:
    AString mProperty;

public:
    PropertyWrapper(const AString& mProperty) : mProperty(mProperty) {

    }
    virtual AString property(const AString& value) = 0;

    AString operator()(const AString& value) {
        return mProperty + " { " + property(value) + " }";
    }
};

struct Url: PropertyWrapper {
private:
public:
    Url(const AString& mProperty) : PropertyWrapper(mProperty) {}

    AString property(const AString& value) override {
        if (value.startsWith("url")) {
            return "\"" + value.substr(5, value.length() - 6) + "\"";
        }
        return "\"" + value.substr(1, value.length() - 2) + "\"";
    }
};
struct Common: PropertyWrapper {
private:
public:
    Common(const AString& mProperty) : PropertyWrapper(mProperty) {}

    AString property(const AString& value) override {
        auto args = value.split(' ');
        AStringVector result;
        for (auto arg : args) {
            if (arg.empty())
                continue;
            if (arg.startsWith("#")) {
                // it's possibly color
                arg = arg.substr(1);
                switch (arg.length()) {
                    case 3: // #fff
                        result << ("0x" + AString(arg[0]) + "0" + arg[1] + "0" + arg[2] + "0" + "_rgb");
                        break;
                    case 4: // #ffff
                        result << ("0x" + AString(arg[3]) + "0" + arg[0] + "0" + arg[1] + "0" + arg[2] + "0" + "_argb");
                        break;
                    case 6: // #ffffff
                        result << ("0x" + arg.substr(0, 2) + arg.substr(2, 2) + arg.substr(4, 2) + "_rgb");
                        break;
                    case 8: // #ffffffff
                        result << ("0x" + arg.substr(6, 2) + arg.substr(0, 2) + arg.substr(2, 2) + arg.substr(4, 2) + "_argb");
                        break;

                    default:
                        throw AException("invalid color: " + arg);
                }
            } else if (isdigit(arg.first()) || arg.first() == '-') {
                // number
                AString suffix;
                if (arg.length() >= 3) {
                    // it's possibly contains literal suffix (px, em, dp)
                    if (isalpha(arg.last())) {
                        suffix = arg.substr(arg.length() - 2, 2);
                        if (suffix == "em")
                            suffix = "dp";
                        suffix = "_" + suffix;
                        arg = arg.substr(0, arg.length() - 2);
                    }
                }
                if (arg == "0")
                    arg = "{}";
                result << (arg + suffix);
            } else {
                // push as is
                result << "\"" + arg + "\"";
            }
        }

        return result.join(", ");
    }
};

void Css2ass::processRule(AVector<AString>& code, const AString& property, const AString& value) {
    const AMap<AString, std::function<AString(const AString&)>> procs = {
            {"-aui-font-rendering", [&](const AString&) {
                return "FontRendering::" + value.uppercase();
            }},
            {"cursor", [&](const AString&) {
                return "ACursor::" + value.uppercase();
            }},
            {"font-family", Url("FontFamily")},
            {"font-size", Common("FontSize")},
            {"box-shadow", Url("BoxShadow")},
            {"border", Url("Border")},
            {"border-radius", Url("BorderRadius")},
            {"padding", Common("Padding")},
            {"margin", Common("Margin")},
            {"color", Common("TextColor")},
            {"background", Common("Background /* !! */")},
            {"width", Common("FixedSize /* width !! */")},
            {"height", Common("FixedSize /* height !! */")},
    };

    if (auto c = procs.contains(property)) {
        code << "        " + c->second(value) + ",";
    } else {
        code << "        //" + property + ": " + value;
        throw AException("unknown property: " + property);
    }
}
