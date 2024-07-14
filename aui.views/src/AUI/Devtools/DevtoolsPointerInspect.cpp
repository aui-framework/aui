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

#include <cstdint>
#include <cstdlib>
#include <range/v3/action.hpp>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

#include "DevtoolsPointerInspect.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <chrono>
#include <variant>

#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Enum/ImageRendering.h"
#include "AUI/Enum/Visibility.h"
#include "AUI/Image/AImage.h"
#include "AUI/Image/APixelFormat.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Model/ATreeModelIndex.h"
#include "AUI/Model/ITreeModel.h"
#include "AUI/Performance/APerformanceFrame.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AUI/Platform/ABaseWindow.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/ARender.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Traits/values.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/Util/ClipOptimizationContext.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"
#include "glm/vector_relational.hpp"

using namespace ass;

static constexpr auto LOG_TAG = "DevtoolsPointerInspect";

namespace {
class FakeContainer: public AViewContainer {
public:
    FakeContainer(_weak<AView> view): mView(std::move(view)) {

    }

    void render(ClipOptimizationContext context) override {
        AViewContainer::render(context);
        auto view = mView.lock();
        if (!view) {
            ARender::string({0, 0}, "Expired");
            return;
        }

        drawView(view, context);
    }


    int getContentMinimumWidth(ALayoutDirection layout) override {
        if (auto view = mView.lock()) {
            return view->getContentMinimumWidth(layout);
        }
        return AViewContainer::getContentMinimumWidth(layout);
    }

    int getContentMinimumHeight(ALayoutDirection layout) override {
        if (auto view = mView.lock()) {
            return view->getContentMinimumHeight(layout);
        }
        return AViewContainer::getContentMinimumHeight(layout);
    }

    [[nodiscard]]
    _<AView> view() {
        return mView.lock();
    }

    void setView(_weak<AView> view) {
        mView = std::move(view);
        AWindow::current()->flagUpdateLayout();
    }

private:
    _weak<AView> mView;
};

class ParentHelper: public AViewContainer {
public:
    ParentHelper(_<FakeContainer> fake): mFake(std::move(fake)) {
        setContents(Centered { mButton = _new<AButton>("Reinflate to parent") });
        connect(mButton->clicked, me::reinflateToParent);
    }

    void render(ClipOptimizationContext context) override {
        AViewContainer::render(context);
        
        auto v = mFake->view();
        mButton->setEnabled(!(v == nullptr || v->getParent() == nullptr));
    }

private:
    _<FakeContainer> mFake;
    _<AButton> mButton;

    void reinflateToParent() {
        auto v = mFake->view();
        if (v == nullptr) {
            return;
        }

        auto parent = v->getParent();
        if (parent == nullptr) {
            return;
        }

        mFake->setView(parent->sharedPtr());
    }
};
}


DevtoolsPointerInspect::DevtoolsPointerInspect(ABaseWindow* targetWindow) : mTargetWindow(targetWindow) {
    using namespace declarative;

    auto address = _new<ATextField>();
    auto resultView = _new<AViewContainer>();

    setContents(Vertical {
        Centered {
          Horizontal {
            Label { "Address (AView*):" },
            address,
            Button { "Inspect" }.clicked(this, [address, resultView] {
                try {
                    auto ptr = [&] {
                        auto ptr = address->text().toStdString();
                        char* end = nullptr;
                        return (AView*)uintptr_t(std::strtoull(ptr.data(), &end, 16));
                    }();
                    if (!ptr) {
                        ALayoutInflater::inflate(resultView, Label { "nullptr" });
                        return;
                    }
                    resultView->setLayout(_new<AVerticalLayout>());
                    resultView->addView(Horizontal { Label { "AReflect::name = " }, Label { AReflect::name(ptr) } });
                    auto fake = _new<FakeContainer>(ptr->sharedPtr());
                    resultView->addView(_new<ParentHelper>(fake));
                    resultView->addView(std::move(fake));
                    AWindow::current()->flagUpdateLayout();
                } catch (const AException& e) {
                    ALayoutInflater::inflate(resultView, Label { e.getMessage() });
                    ALogger::err(LOG_TAG) << "Unable to inspect: " << e;
                }
            }),
          },
        },
        Centered { resultView },
    });
}
