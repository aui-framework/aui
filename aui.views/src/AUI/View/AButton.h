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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once
#include "AView.h"
#include "AUI/Common/AString.h"
#include "AUI/Render/Render.h"
#include "ALabel.h"
#include "AUI/ASS/Selector/AAssSelector.h"

/**
 * <p>Represents a simple button with text, which can be pushed to make some action.</p>
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/AButton.gif">
 */
class API_AUI_VIEWS AButton: public ALabel
{	
public:
	AButton();
	explicit AButton(const AString& text);
	virtual ~AButton() = default;

    [[nodiscard]]
    bool isDefault() const noexcept {
        return mDefault;
    }

	void setDefault(bool isDefault = true);

    bool consumesClick(const glm::ivec2& pos) override;

signals:
	emits<bool> defaultState;
	emits<> becameDefault;
	emits<> noLongerDefault;

private:
	Watchable<bool> mDefault = Watchable<bool>(defaultState, becameDefault, noLongerDefault);
};


namespace ass::Button {
    struct Default: IAssSubSelector {
    private:
        _unique<IAssSubSelector> mWrapped;

    public:
        template<typename T>
        Default(T value): mWrapped(new T(std::move(value))) {}


        bool isPossiblyApplicable(AView* view) override {
            return mWrapped->isPossiblyApplicable(view) && dynamic_cast<AButton*>(view);
        }

        bool isStateApplicable(AView* view) override {
            if (!mWrapped->isStateApplicable(view))
                return false;

            if (auto c = dynamic_cast<AButton*>(view)) {
                return c->isDefault();
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            IAssSubSelector::setupConnections(view, helper);
            mWrapped->setupConnections(view, helper);

            if (auto c = dynamic_cast<AButton*>(view)) {
                c->defaultState.clearAllConnectionsWith(helper.get());
                AObject::connect(c->defaultState, slot(helper)::onInvalidateStateAss);
            }
        }
    };
}