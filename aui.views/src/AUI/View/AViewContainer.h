/*
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

#include <AUI/Views.h>
#include "AUI/Common/SharedPtr.h"
#include <glm/glm.hpp>
#include "AUI/Layout/ALayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"

/**
 * @brief A view that represents a set of views.
 * @ingroup useful_views
 * @details
 * AViewContainer can store, render, resize, provide events to and handle the child views.
 *
 * AViewContainer does not control the position and size of the child views by itself; instead, it delegates that
 * responsibility to it's @ref layout_managers "layout manager".
 *
 * Since AViewContainer is an instance of AView, AViewContainer can handle AViewContainers recursively, thus, making
 * possible complex UI by nested AViewContainers with different layout managers.
 */
class API_AUI_VIEWS AViewContainer: public AView
{
public:
	AViewContainer();
	virtual ~AViewContainer();
	void addView(const _<AView>& view);
	void addView(size_t index, const _<AView>& view);
	void removeView(const _<AView>& view);
	void removeView(AView* view);
	void removeView(size_t index);
	void removeAllViews();

	void render() override;

    void focus() override;

    void onMouseEnter() override;
	void onMouseMove(glm::ivec2 pos) override;
	void onMouseLeave() override;

    void onDpiChanged() override;


    int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;
	
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
	void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;
	void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    void onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) override;

    bool onGesture(const glm::ivec2 &origin, const AGestureEvent &event) override;

    bool consumesClick(const glm::ivec2& pos) override;
    void setSize(int width, int height) override;
    void setEnabled(bool enabled = true) override;

    auto begin() const {
        return mViews.cbegin();
    }
    auto end() const {
        return mViews.cend();
    }

	/**
	 * @brief Set new layout manager for this AViewContainer. DESTROYS OLD LAYOUT MANAGER WITH ITS VIEWS!!!
	 */
	void setLayout(_<ALayout> layout);
	_<ALayout> getLayout() const;

	virtual _<AView> getViewAt(glm::ivec2 pos, bool ignoreGone = true);
	_<AView> getViewAtRecursive(glm::ivec2 pos);

	template<typename T>
	_<T> getViewAtRecursiveOf(glm::ivec2 pos, bool ignoreGone = true) {
		for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
		{
			auto view = *it;
			auto targetPos = pos - view->getPosition();

			if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.x < view->getSize().x && targetPos.y < view->getSize().y)
			{
				if (!ignoreGone || view->getVisibility() != Visibility::GONE) {
					if (auto applicable = _cast<T>(view))
						return applicable;
					if (auto container = _cast<AViewContainer>(view)) {
						if (auto applicable = container->getViewAtRecursiveOf<T>(targetPos, ignoreGone)) {
							return applicable;
						}
					}
				}
			}
		}
        return nullptr;
	}

	virtual void updateLayout();

    const AVector<_<AView>>& getViews() const
	{
		return mViews;
	}

    void addViewCustomLayout(const _<AView>& view);

protected:
    AVector<_<AView>> mViews;

    void drawView(const _<AView>& view);

    template <typename Iterator>
    void drawViews(Iterator begin, Iterator end)
    {
        for (auto i = begin; i != end; ++i)
        {
            drawView(*i);
        }
    }

    void recompileAss() override;


    /**
     * @brief Updates layout of the parent AViewContainer if size of this AViewContainer was changed.
     */
    virtual void updateParentsLayoutIfNecessary();


    /**
     * @brief Moves (like via std::move) all children and layout of the specified container to this container.
     * @param container container. Must be pure AViewContainer (cannot be a derivative from AViewContainer).
     * @note If access to this function is restricted or you want to pass an object derived from AViewContainer, you
     * should use ALayoutInflater::inflate instead.
     */
    void setContents(const _<AViewContainer>& container);

private:
    _<ALayout> mLayout;
    bool mSizeSet = false;
    glm::ivec2 mPreviousSize = mSize;

    void notifyParentEnabledStateChanged(bool enabled) override;

};
