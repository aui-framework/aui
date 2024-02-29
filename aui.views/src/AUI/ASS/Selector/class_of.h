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
        struct ClassOf: IAssSubSelector {
        private:
            AStringVector mClasses;
            
        public:
            ClassOf(const AStringVector& classes) : mClasses(classes) {}
            ClassOf(const AString& clazz) : mClasses({clazz}) {}

            bool isPossiblyApplicable(AView* view) override {
                for (auto& v : mClasses) {
                    if (view->getAssNames().contains(v)) {
                        return true;
                    }
                }
                return false;
            }

            bool isStateApplicable(AView* view) override {
                return isPossiblyApplicable(view);
            }

            const AStringVector& getClasses() const {
                return mClasses;
            }
        };
    }

    struct class_of: detail::ClassOf {
    public:
        class_of(const AStringVector& classes) : ClassOf(classes) {}
        class_of(const AString& clazz) : ClassOf(clazz) {}

        using hover = ass::hovered<detail::ClassOf>;
        using active = ass::activated<detail::ClassOf>;
        using focus = ass::focused<detail::ClassOf>;
        using disabled = ass::disabled<detail::ClassOf>;
    };

    using c = class_of;
}