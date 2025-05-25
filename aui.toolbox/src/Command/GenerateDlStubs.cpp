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

#include <range/v3/all.hpp>
#include "GenerateDlStubs.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/IO/AFileOutputStream.h"

AString GenerateDlStubs::getName() { return "generate_dl_stubs"; }

AString GenerateDlStubs::getSignature() { return "<sigs file> <output file>"; }

AString GenerateDlStubs::getDescription() {
    return "generates dlopen/dlsyms stubs file from a file containing list of signatures.";
}

void GenerateDlStubs::run(Toolbox& t) {
    if (t.args.size() != 2) {
        throw IllegalArgumentsException("requires 2 arguments");
    }

    auto& inputFile = t.args[0];
    auto& outputFile = t.args[1];

    ATokenizer tokenizer(_new<AFileInputStream>(inputFile));
    AFileOutputStream fos(outputFile);
    while (!tokenizer.isEof()) {
        auto contents = tokenizer.readStringUntilUnescaped('\n');
        auto view = std::string_view(contents);
        auto take = [&](size_t chars) {
            auto result = view.substr(0, chars);
            view = view.substr(chars + 1);
            return result;
        };

        if (contents.empty()) {
            fos << '\n';
            continue;
        }
        if (contents.starts_with("// aui.toolbox: ")) {
            take(view.find(':') + 1);
            fos << view;
            fos << '\n';
            continue;
        }
        if (contents.starts_with("//")) {
            continue;
        }

        auto returnT = take(view.rfind(' ', view.find('(')));
        auto name = take(view.find('('));
        auto arguments = take(view.find(')'));
        if (arguments == "void") {
            arguments = "";
        }

        struct Argument {
            std::string_view type;
            std::string_view name;
        };

        auto argumentsList = arguments
            | ranges::views::split(',')
            | ranges::views::transform([](auto&& rng) -> Argument {
                   const auto size = ranges::distance(rng);
                   if (size == 0) {
                       return {};
                   }
                   auto argumentStr = std::string_view(&*rng.begin(), size);
                   auto it = ranges::distance(argumentStr.begin(), ranges::find_if(argumentStr | ranges::views::reverse, [](char c) {
                       switch (c) {
                           case ' ':
                           case '*':
                           case '&':
                               return true;
                           default:
                               return false;
                       }
                   }).base());
                   return {
                       .type = argumentStr.substr(0, it),
                       .name = argumentStr.substr(it),
                   };
               });

        fos << returnT << ' ' << name << '(' << arguments << ") {\n";
        fos << "    " << "static auto auiFuncPtr = reinterpret_cast<" << returnT << "(*)(" << arguments << ")>(dlsym(handle, \"" << name << "\"));\n";
        fos << "    " << "return (*auiFuncPtr)(";
        {
            bool first = true;
            for (auto argument : argumentsList) {
                if (first) {
                    first = false;
                } else {
                    fos << ", ";
                }
                fos << argument.name;
            }
        }
        fos << ");\n";
        fos << "}\n";
        fos << '\n';
    }

    std::cout << inputFile << " -> " << outputFile << std::endl;
}
