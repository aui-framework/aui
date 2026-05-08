/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Util/ALayoutInflater.h>
#include "AScrollAreaViewport.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/View/AView.h"

static constexpr auto RENDER_TO_TEXTURE_THRESHOLD_AREA = 128 * 128;

class AScrollAreaViewport::Inner: public AViewContainer {
public:
    Inner() {
        addAssName("AScrollAreaViewportInner");
    }
};

AScrollAreaViewport::AScrollAreaViewport() {
  addAssName("AScrollAreaViewport");
  addView(mInner = _new<Inner>());
}

AScrollAreaViewport::~AScrollAreaViewport() {

}

void AScrollAreaViewport::setContents(_<AView> content) {
  ALayoutInflater::inflate(mInner, content);
  mContents = std::move(content);
  mInner->setSkipUntilLayoutUpdate(false);
  mInner->setPosition(-glm::ivec2(mScroll));
  requestLayout();
  redraw();
}

void AScrollAreaViewport::setScrollSurfaceSize(glm::ivec2 size) {
  size = glm::max(size, glm::ivec2(0));
  if (mScrollSurfaceSize == size) {
    return;
  }
  mScrollSurfaceSize = size;
  requestLayout();
  redraw();
}

glm::ivec2 AScrollAreaViewport::onIntrinsicMeasure(AConstraints constraints) {
  return mInner->measure(constraints);
}

void AScrollAreaViewport::applyGeometryToChildren() {
  mInner->setSkipUntilLayoutUpdate(false);
  mInner->layout(-glm::ivec2(mScroll), mScrollSurfaceSize);
  if (mInner->getSize().x * mInner->getSize().y >= RENDER_TO_TEXTURE_THRESHOLD_AREA) {
    if (!IRenderViewToTexture::isEnabledForView(*mInner)) {
      auto w = AWindow::current();
      if (!w) {
        return;
      }

      IRenderViewToTexture::enableForView(w->getRenderingContext()->renderer(), *mInner);
    }
  } else {
    IRenderViewToTexture::disableForView(*mInner);
  }
}
void AScrollAreaViewport::updateContentsScroll() {
  mInner->setSkipUntilLayoutUpdate(false);
  mInner->setPosition(-glm::ivec2(mScroll));
  emit mScrollChanged(mScroll);
  redraw();
}
