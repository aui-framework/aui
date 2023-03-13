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
// Created by alex2 on 07.01.2021.
//


#include "Auisl.h"

#include <AUI/IO/APath.h>
#include "Auisl.h"
#include "ShadingLanguage/Lang/Frontend/IFronted.h"
#include "ShadingLanguage/Lang/Frontend/CppFrontend.h"
#include "ShadingLanguage/Lang/Frontend/GLSLFrontend.h"
#include "AUI/IO/AStringStream.h"
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
                                                                   | view::keys
                                                                   | view::join(',')
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
    const auto& output = t.args[2];
    APath(output).touch();

}
