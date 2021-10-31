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


#include "AViewContainer.h"
#include "AScrollbar.h"

class AScrollAreaContainer;

class API_AUI_VIEWS AScrollArea: public AViewContainer {
public:
    class Builder;

private:
    _<AScrollAreaContainer> mContentContainer;
    _<AScrollbar> mVerticalScrollbar;
    _<AScrollbar> mHorizontalScrollbar;

    explicit AScrollArea(const Builder& builder);
public:
    AScrollArea();
    virtual ~AScrollArea();

    _<AViewContainer> getContentContainer() const;
    int getContentMinimumHeight() override;
    void setSize(int width, int height) override;
    void onMouseWheel(glm::ivec2 pos, int delta) override;
    void setContents(const _<AViewContainer>& container);

    class Builder {
    friend class AScrollArea;
    private:
        _<AScrollbar> mExternalVerticalScrollbar;
        _<AViewContainer> mContents;
        bool mExpanding = false;

    public:
        Builder() = default;

        Builder& withExternalVerticalScrollbar(const _<AScrollbar>& externalVerticalScrollbar) {
            mExternalVerticalScrollbar = externalVerticalScrollbar;
            return *this;
        }
        Builder& withContents(const _<AViewContainer>& contents) {
            mContents = contents;
            return *this;
        }

        Builder& withExpanding() {
            mExpanding = true;
            return *this;
        }


        _<AScrollArea> build() {
            return {new AScrollArea(*this) };
        }

        operator _<AView>() {
            return build();
        }
        operator _<AViewContainer>() {
            return build();
        }
    };
};

