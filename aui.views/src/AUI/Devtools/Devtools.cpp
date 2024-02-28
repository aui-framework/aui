// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Reflect/AReflect.h>
#include <AUI/View/AViewContainer.h>
#include "Devtools.h"

AString Devtools::prettyViewName(AView* view) {
    AString name;
    if (auto c = dynamic_cast<AViewContainer*>(view)) {
        if (c->getLayout()) {
            name = AReflect::name(c->getLayout().get());
        } else {
            name = "<no layout>";
        }
        name = "{} ({})"_format(name, AReflect::name(view));
    } else {
        name = AReflect::name(view);
    }

    // remove 'class '
    if (name.startsWith("class ")) {
        name = name.substr(6);
    }
    if (name.startsWith("LayoutVisitor::")) {
        name = "A" + name.substr(15);
    }
    return name;
}
