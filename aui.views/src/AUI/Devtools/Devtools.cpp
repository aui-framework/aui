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
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Reflect/AReflect.h>
#include <AUI/View/AViewContainer.h>
#include "Devtools.h"

#include <range/v3/all.hpp>

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
    auto filter = [](const AString& name) { return !name.contains("::detail::"); };
    if (auto rng = view->getAssNames() | ranges::view::filter(filter); !ranges::empty(rng)) {
        name = ranges::back(rng);
    }
    return name;
}
