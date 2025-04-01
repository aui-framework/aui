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

//
// Created by alex2 on 07.01.2021.
//


#include "Auisl.h"

#include <AUI/IO/APath.h>
#include "Auisl.h"
#include "ShadingLanguage/Lang/Frontend/IFrontend.h"
#include "ShadingLanguage/Lang/Frontend/CppFrontend.h"
#include "ShadingLanguage/Lang/Frontend/GLSLFrontend.h"
#include "AUI/IO/AStringStream.h"
#include "ShadingLanguage/Lang/SL.h"
#include <AUI/Traits/strings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Util/ARandom.h>
#include <range/v3/view.hpp>

namespace {
    template<aui::derived_from<IFrontend> T>
    struct Factory {
        _unique<IFrontend> operator()() const {
            return std::make_unique<T>();
        }
    };
}

static const AMap<AString, std::function<_unique<IFrontend>()>>& targetPlatforms() {
    static const AMap<AString, std::function<_unique<IFrontend>()>> t = {
        {"software", Factory<CppFrontend>{} },
        {"glsl120", Factory<GLSLFrontend>{} },
    };
    return t;
};

AString Auisl::getName() {
    return "auisl";
}

AString Auisl::getSignature() {
    using namespace ranges;
    return "<target_platform {}> <auisl_file> <output_cpp>"_format(targetPlatforms()
                                                                   | views::keys
                                                                   | views::join(',')
                                                                   | to<AString>());
}

AString Auisl::getDescription() {
    return "invokes AUI ShadingLanguage compiler on the specified file.";
}

void Auisl::run(Toolbox& t) {
    if (t.args.size() != 3) {
        throw IllegalArgumentsException("auisl requires 3 args");
    }
    const auto& targetPlatform = t.args[0];

    auto frontend = targetPlatforms()[targetPlatform]();

    APath input = t.args[1];
    APath output = t.args[2];

    frontend->setShaderType(input.endsWith(".vsh") ? ShaderType::VERTEX : ShaderType::FRAGMENT);
    frontend->parseShader(aui::sl::parseCode(_new<AFileInputStream>(input), input.parent()));
    frontend->writeCpp(output);
    if (frontend->hasErrors()) {
        throw AException("frontend has errors");
    }
}
