/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>
#include <AUI/Util/kAUI.h>
#include <utility>

class AView;


namespace ass {

    class API_AUI_VIEWS IAssSubSelector {
    public:
        virtual bool isPossiblyApplicable(AView* view) = 0;
        virtual bool isStateApplicable(AView* view);
        virtual void setupConnections(AView* view, const _<AAssHelper>& helper);
        virtual ~IAssSubSelector() = default;
    };

    class AAssSelector {
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
            for (auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view))
                    return true;
            }
            return false;
        }
        bool isStateApplicable(AView* view) const {
            for (auto& s : mSubSelectors) {
                if (s->isStateApplicable(view))
                    return true;
            }
            return false;
        }
        void setupConnections(AView* view, const _<AAssHelper>& helper) const {
            for (auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view)) {
                    s->setupConnections(view, helper);
                }
            }
        }
        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void addSubSelector(SubSelector&& subSelector) {
            processSubSelector(std::forward<SubSelector>(subSelector));
        }

    };

    using sel = AAssSelector;
}