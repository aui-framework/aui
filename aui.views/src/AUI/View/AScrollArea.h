// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once


#include "AViewContainer.h"
#include "AScrollbar.h"

class AScrollAreaContainer;

/**
 * @brief A container with vertical and horizontal scrollbars.
 * @ingroup useful_views
 */
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
    int getContentMinimumHeight(ALayoutDirection layout) override;
    void setSize(glm::ivec2 size) override;
    void setContents(const _<AViewContainer>& container);

    void scroll(int deltaByX, int deltaByY) noexcept {
        AUI_NULLSAFE(mHorizontalScrollbar)->scroll(deltaByX);
        AUI_NULLSAFE(mVerticalScrollbar)->scroll(deltaByY);
    }

    bool onGesture(const glm::ivec2 &origin, const AGestureEvent &event) override;

    void onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) override;

    class Builder {
    friend class AScrollArea;
    private:
        _<AScrollbar> mExternalVerticalScrollbar;
        _<AViewContainer> mContents;
        bool mExpanding = false;

    public:
        Builder() = default;

        Builder& withExternalVerticalScrollbar(_<AScrollbar> externalVerticalScrollbar) {
            mExternalVerticalScrollbar = std::move(externalVerticalScrollbar);
            return *this;
        }
        Builder& withContents(_<AViewContainer> contents) {
            mContents = std::move(contents);
            return *this;
        }

        Builder& withExpanding() {
            mExpanding = true;
            return *this;
        }


        _<AScrollArea> build() {
            return aui::ptr::manage(new AScrollArea(*this));
        }

        operator _<AView>() {
            return build();
        }
        operator _<AViewContainer>() {
            return build();
        }
    };
};

