// AUI Framework - Declarative UI toolkit for modern C++17
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
#include "active.h"
#include "focus.h"
#include "disabled.h"

namespace ass {

    namespace detail {
        struct debug_selector : virtual IAssSubSelector {
        public:
            debug_selector() = default;

            bool isStateApplicable(AView* view) override {
                return false;
            }

            bool isPossiblyApplicable(AView* view) override {
                return false;
            }
        };
    }

    struct debug_selector: detail::debug_selector {
    public:
        debug_selector() = default;

        using hover = ass::hovered<detail::debug_selector>;
        using active = ass::active<detail::debug_selector>;
        using focus = ass::focus<detail::debug_selector>;
        using disabled = ass::disabled<detail::debug_selector>;
    };

}