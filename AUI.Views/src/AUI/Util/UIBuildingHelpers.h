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
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Layout/AGridLayout.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/i18n/AI18n.h>


template<typename Layout, typename... Args>
inline auto _container(const AVector<_<AView>>& views, Args&&... args)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<Layout>(args...));

	for (const auto& v : views)
		c->addView(v);

	return c;
}

inline auto _form(const AVector<std::pair<std::variant<AString, _<AView>>, _<AView>>>& views)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<AAdvancedGridLayout>(2, views.size()));

	for (const auto& v : views) {
		try {
			c->addView(_new<ALabel>(std::get<AString>(v.first)));
		}
		catch (const std::bad_variant_access&) {
			c->addView(std::get<_<AView>>(v.first));
		}
		c->addView(v.second);
	}

	return c;
}
