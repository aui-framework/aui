/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#include <AUI/View/ASpacerExpanding.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/Layout/AWordWrappingLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Layout/AGridLayout.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Layout/AAbsoluteLayout.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Traits/strings.h>
#include "Declarative.h"


template<typename Layout, typename... Args>
inline auto _container(AVector<_<AView>> views, Args&&... args)
{
    auto c = _new<AViewContainer>();
    c->setLayout(std::make_unique<Layout>(std::forward<Args>(args)...));

    c->setViews(std::move(views));

    return c;
}

inline auto _form(const AVector<std::pair<std::variant<AString, _<AView>>, _<AView>>>& views)
{
	auto c = _new<AViewContainer>();
	c->setLayout(std::make_unique<AAdvancedGridLayout>(2, views.size()));
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


/**
 * Places views in a column.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/vertical.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AVerticalLayout</dt>
 *  </dl>
 * </p>
 */
using Vertical = aui::ui_building::layouted_container_factory<AVerticalLayout>;

/**
 * Places views in a row.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/horizontal.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AHorizontalLayout</dt>
 *  </dl>
 * </p>
 */
using Horizontal = aui::ui_building::layouted_container_factory<AHorizontalLayout>;

/**
 * Places views in a stack, centering them.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AStackedLayout</dt>
 *  </dl>
 * </p>
 */
using Stacked = aui::ui_building::layouted_container_factory<AStackedLayout>;

/**
 * Places views according to specified xy coordinates.
 * <p>
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AAbsoluteLayout</dt>
 *  </dl>
 * </p>
 */
using Absolute = aui::ui_building::layouted_container_factory<AAbsoluteLayout>;

/**
 * Does not actually set the layout. The views' geometry is determined manually.
 * @deprecated Use AAbsoluteLayout instead.
 * <p>
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> null</dt>
 *  </dl>
 * </p>
 */
using CustomLayout = aui::ui_building::layouted_container_factory<std::nullopt_t>;



/**
 * <p>
 * <code>Center</code> is an alias to Stacked. When Stacked is used only for centering views, you can use
 * this alias in order to improve understanding of your code.
 * </p>
 */
using Centered = Stacked;