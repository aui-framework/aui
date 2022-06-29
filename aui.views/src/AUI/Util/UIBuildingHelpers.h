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
#include "AUI/Common/AVector.h"
#include "AUI/View/AView.h"
#include "AUI/Layout/AGridLayout.h"
#include <variant>
#include "AUI/View/AViewContainer.h"
#include "AUI/View/ALabel.h"
#include <AUI/Common/SharedPtr.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/AMetric.h>
#include <AUI/View/ASpacer.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/Layout/AWordWrappingLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Layout/AGridLayout.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Traits/strings.h>


template<typename Layout, typename... Args>
inline auto _container(AVector<_<AView>> views, Args&&... args)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<Layout>(std::forward<Args>(args)...));

    c->setViews(std::move(views));

	return c;
}

inline auto _form(const AVector<std::pair<std::variant<AString, _<AView>>, _<AView>>>& views)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<AAdvancedGridLayout>(2, views.size()));
	c->setExpanding({2, 0});
	for (const auto& v : views) {
		try {
			c->addView(_new<ALabel>(std::get<AString>(v.first)));
		}
		catch (const std::bad_variant_access&) {
			c->addView(std::get<_<AView>>(v.first));
		}
		v.second->setExpanding({2, 0});
		c->addView(v.second);
	}

	return c;
}

namespace aui::detail {
    template<typename Layout>
    struct container_helper {
    private:
        AVector<_<AView>> mViews;

    public:

        struct Expanding {
        private:
            AVector<_<AView>> mViews;

        public:
            Expanding(std::initializer_list<_<AView>> views) {
                mViews.reserve(views.size());
                for (auto& v : views) {
                    if (v) {
                        mViews << v;
                    }
                }
            }

            operator _<AView>() {
                return (_container<Layout>(std::move(mViews)) let {it->setExpanding();});
            }
            operator _<AViewContainer>() {
                return (_container<Layout>(std::move(mViews)) let {it->setExpanding();});
            }
            _<AViewContainer> operator<<(const AString& assEntry) {
                return (_container<Layout>(std::move(mViews)) let {it->setExpanding();}) << assEntry;
            }
            template<typename T>
            _<AViewContainer> operator^(const T& t) {
                return (_container<Layout>(std::move(mViews)) let {it->setExpanding();}) ^ t;
            }
            template<typename T>
            _<AViewContainer> operator+(const T& t) {
                return (_container<Layout>(std::move(mViews)) let {it->setExpanding();}) + t;
            }

            operator _<AView>() const {
                return (_container<Layout>(mViews) let {it->setExpanding();});
            }
            operator _<AViewContainer>() const {
                return (_container<Layout>(mViews) let {it->setExpanding();});
            }
            _<AViewContainer> operator<<(const AString& assEntry) const {
                return (_container<Layout>(mViews) let {it->setExpanding();}) << assEntry;
            }
            template<typename T>
            _<AViewContainer> operator^(const T& t) const {
                return (_container<Layout>(mViews) let {it->setExpanding();}) ^ t;
            }
            template<typename T>
            _<AViewContainer> operator+(const T& t) const {
                return (_container<Layout>(mViews) let {it->setExpanding();}) + t;
            }
        };


        container_helper(std::initializer_list<_<AView>> views) {
            mViews.reserve(views.size());
            for (auto& v : views) {
                if (v) {
                    mViews << v;
                }
            }
        }
        operator _<AView>() const {
            return _container<Layout>(mViews);
        }
        operator _<AViewContainer>() const {
            return _container<Layout>(mViews);
        }
        _<AViewContainer> operator<<(const AString& assEntry) const {
            return _container<Layout>(mViews) << assEntry;
        }
        template<typename T>
        _<AViewContainer> operator^(const T& t) const {
            return _container<Layout>(mViews) ^ t;
        }
        template<typename T>
        _<AViewContainer> operator+(const T& t) const {
            return _container<Layout>(mViews) + t;
        }

        _<AViewContainer> operator->() const {
            return _container<Layout>(mViews);
        }
    };
}

/**
 * Places views in a column.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/vertical.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AVerticalLayout">AVerticalLayout</a></dt>
 *  </dl>
 * </p>
 */
using Vertical = aui::detail::container_helper<AVerticalLayout>;

/**
 * Places views in a row.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/horizontal.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AHorizontalLayout">AHorizontalLayout</a></dt>
 *  </dl>
 * </p>
 */
using Horizontal = aui::detail::container_helper<AHorizontalLayout>;

/**
 * Places views in a stack, centering them.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AStackedLayout">AStackedLayout</a></dt>
 *  </dl>
 * </p>
 */
using Stacked = aui::detail::container_helper<AStackedLayout>;

/**
 * <p>
 * <code>Center</code> is an alias to <a href="#Stacked">Stacked</a>. When <a href="#Stacked">Stacked</a> is used only for centering views, you can use
 * this alias in order to improve understanding of your code.
 * </p>
 */
using Centered = Stacked;