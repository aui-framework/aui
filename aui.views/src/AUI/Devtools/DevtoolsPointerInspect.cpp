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
#include "AUI/Common/AStringVector.h"
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
#include "AUI/Platform/AWindowBase.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Traits/values.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/Render/ARenderContext.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"
#include "glm/vector_relational.hpp"

using namespace ass;
using namespace declarative;

static constexpr auto LOG_TAG = "DevtoolsPointerInspect";

namespace {
class FakeContainer: public AViewContainerBase {
public:
    FakeContainer(_weak<AView> view): mView(std::move(view)) {
        connect(redrawn, [&] {
            if (mRedrawLock.is_locked()) {
                return;
            }
            redraw();
        });
    }

    void render(ARenderContext ctx) override {
        std::unique_lock lock(mRedrawLock);
        AViewContainerBase::render(ctx);
        auto view = mView.lock();
        if (!view) {
            ctx.render.string({0, 10}, "Expired");
            return;
        }

        drawView(view, ctx);
    }

    int getContentMinimumWidth() override {
        if (auto view = mView.lock()) {
            return view->getContentMinimumWidth();
        }
        return AViewContainerBase::getContentMinimumWidth();
    }

    int getContentMinimumHeight() override {
        if (auto view = mView.lock()) {
            return view->getContentMinimumHeight();
        }
        return AViewContainerBase::getContentMinimumHeight();
    }

    [[nodiscard]]
    _<AView> view() {
        return mView.lock();
    }

    void setView(_weak<AView> view) {
        mView = std::move(view);
        markMinContentSizeInvalid();
    }

private:
    ASpinlockMutex mRedrawLock;
    _weak<AView> mView;
};

class ParentHelper: public AViewContainerBase {
public:
    ParentHelper(_<FakeContainer> fake): mFake(std::move(fake)) {
        setContents(Centered { mButton = _new<AButton>("Reinflate to parent") });
        connect(mButton->clicked, me::reinflateToParent);
    }

    void render(ARenderContext context) override {
        AViewContainerBase::render(context);

        auto v = mFake->view();
        mButton->setEnabled(!(v == nullptr || v->getParent() == nullptr));
    }
signals:
    emits<AView*> reinflate;

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

        emit reinflate(dynamic_cast<AView*>(parent));
    }
};
}

using namespace declarative;

DevtoolsPointerInspect::DevtoolsPointerInspect(AWindowBase* targetWindow) : mTargetWindow(targetWindow) {
    setContents(Vertical {
        Centered {
          Horizontal {
            Label { "Address (AView*):" },
            mAddress,
            Button { .content = Label { "Inspect" }, .onClick = [this] {
                try {
                    auto ptr = [&] {
                        auto ptr = (*mAddress->text()).toStdString();
                        char* end = nullptr;
                        return (AView*)uintptr_t(std::strtoull(ptr.data(), &end, 16));
                    }();
                    if (!ptr) {
                        ALayoutInflater::inflate(mResultView, Label { "nullptr" });
                        return;
                    }
                    inspect(ptr);
                } catch (const AException& e) {
                    ALayoutInflater::inflate(mResultView, Label { e.getMessage() });
                    ALogger::err(LOG_TAG) << "Unable to inspect: " << e;
                }
            }},
          },
        },
        Centered { mResultView },
    });
}

void DevtoolsPointerInspect::inspect(AView* ptr) {
    ALogger::info(LOG_TAG) << "Inspecting: " << ptr;
    mResultView->setLayout(std::make_unique<AVerticalLayout>());
    mResultView->addView(Horizontal { Label { "AReflect::name = " }, Label { AReflect::name(ptr) } });
    mResultView->addView(Horizontal { Label { "Ass names = " }, Label { AStringVector(ptr->getAssNames()).join(", ") } });
    auto fake = _new<FakeContainer>(aui::ptr::shared_from_this(ptr));
    auto parentHelper = _new<ParentHelper>(fake);
    connect(parentHelper->reinflate, [this](AView* ptr) {
        mAddress->setText("{}"_format((void*)ptr));
        inspect(ptr);
    });
    mResultView->addView(parentHelper);
    mResultView->addView(std::move(fake));
    markMinContentSizeInvalid();
}
