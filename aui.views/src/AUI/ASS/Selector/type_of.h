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
// Created by alex2 on 02.01.2021.
//

#pragma once

#include <AUI/Util/kAUI.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

#include "hovered.h"
#include "activated.h"
#include "focused.h"
#include "disabled.h"

namespace ass {

    namespace detail {
        template<typename T>
        struct Type : IAssSubSelector {
            static_assert(std::is_polymorphic_v<T>, "Your type is not polymorphic! Please define at least virtual "
                                                    "destructor for using type_of (t) style selector "
                                                    "(see https://en.cppreference.com/w/cpp/language/object#Polymorphic_objects)");
        public:
            bool isPossiblyApplicable(AView* view) override {
                return dynamic_cast<T*>(view) != nullptr;
            }

        };
    }

    template<typename T>
    struct type_of: detail::Type<T> {
    public:

        using hover = ass::hovered<detail::Type<T>>;
        using active = ass::activated<detail::Type<T>>;
        using focus = ass::activated<detail::Type<T>>;
        using disabled = ass::disabled<detail::Type<T>>;
    };

    template<typename T>
    using t = type_of<T>;
}