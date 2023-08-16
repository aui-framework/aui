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
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Util/kAUI.h>
#include <utility>

class AView;
class AAssHelper;

namespace ass {

    class API_AUI_VIEWS IAssSubSelector {
    public:
        virtual bool isPossiblyApplicable(AView* view) = 0;
        virtual bool isStateApplicable(AView* view);
        virtual void setupConnections(AView* view, const _<AAssHelper>& helper);
        virtual ~IAssSubSelector() = default;
    };

    class API_AUI_VIEWS AAssSelector {
    private:
        AVector<_<IAssSubSelector>> mSubSelectors;

        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _new<SubSelector>(std::forward<SubSelector>(subSelector));
        }

        template<typename SubSelector, std::enable_if_t<std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _<IAssSubSelector>(subSelector);
        }

        template<typename SubSelector, typename...SubSelectors>
        void processSubSelectors(SubSelector&& subSelector, SubSelectors&&... subSelectors) {
            processSubSelector(std::forward<SubSelector>(subSelector));
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }

        void processSubSelectors() {}

    public:
        template<typename...SubSelectors>
        AAssSelector(SubSelectors&&... subSelectors) {
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }
        AAssSelector(AAssSelector&& move): mSubSelectors(std::move(move.mSubSelectors)) {

        }
        explicit AAssSelector(std::nullptr_t) {}
        AAssSelector(const AAssSelector&) = default;

        ~AAssSelector() {
            mSubSelectors.clear();
        }

        AAssSelector& operator=(const AAssSelector& c) = default;

        bool isPossiblyApplicable(AView* view) const {
            for (const auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view)) {
                    return true;
                }
            }
            return false;
        }
        bool isStateApplicable(AView* view) const {
            for (const auto& s : mSubSelectors) {
                if (s->isStateApplicable(view)) {
                    return true;
                }
            }
            return false;
        }
        void setupConnections(AView* view, const _<AAssHelper>& helper) const;
        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void addSubSelector(SubSelector&& subSelector) {
            processSubSelector(std::forward<SubSelector>(subSelector));
        }

        [[nodiscard]]
        const AVector<_<IAssSubSelector>>& getSubSelectors() const {
            return mSubSelectors;
        }
    };

    using sel = AAssSelector;
}