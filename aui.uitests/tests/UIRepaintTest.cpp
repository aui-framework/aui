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

#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "AUI/ASS/Property/Expanding.h"
#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/ASS/Property/LayoutSpacing.h"
#include "AUI/Test/UI/By.h"
#include "AUI/Util/ALayoutInflater.h"

using namespace declarative;

namespace {
    class UIRepaintTest : public testing::UITest {
    public:
    protected:
        void SetUp() override {
            UITest::SetUp();
        }

        void inflate(_<AView> view) {
            mWindow = _new<AWindow>();
            using namespace declarative;
            ALayoutInflater::inflate(mWindow, std::move(view));
            mWindow->show();
        }

        void TearDown() override {
            mWindow = nullptr;
            mView = nullptr;
            UITest::TearDown();
        }

        _<AWindow> mWindow;
        _<AView> mView;
    };

    class ViewContainerMock: public AViewContainer {
    public:
        ViewContainerMock() {
            ON_CALL(*this, markPixelDataInvalid).WillByDefault([this](const auto& a) {
                return AViewContainer::markPixelDataInvalid(a);
            });
        }
        MOCK_METHOD(void, markPixelDataInvalid, (ARect<int> invalidArea), (override));
    };
}

static constexpr auto BASE_REPAINT_COUNT = 2;

TEST_F(UIRepaintTest, ContainerRepaintTest1) {
    auto label = _new<ALabel>("test");
    auto container = _new<ViewContainerMock>();

    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(BASE_REPAINT_COUNT);
        container->setLayout(_new<AHorizontalLayout>());
        container->addView(label);
        inflate(Centered { container });
        AUI_REPEAT(10) { uitest::frame(); }
    }
}

TEST_F(UIRepaintTest, ContainerRepaintTest2) {
    testing::InSequence s;
    auto label = _new<ALabel>("test");
    auto container = _new<ViewContainerMock>();

    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(BASE_REPAINT_COUNT);
        container->setLayout(_new<AHorizontalLayout>());
        container->addView(label);
        inflate(Centered { container });
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(testing::AtLeast(1));
        label->setText("ololo");
        AUI_REPEAT(10) { uitest::frame(); }
    }
}

TEST_F(UIRepaintTest, ContainerRepaintTest3) {
    auto label = _new<ALabel>("test");
    auto container = _new<ViewContainerMock>();

    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(BASE_REPAINT_COUNT);

        container->setLayout(_new<AHorizontalLayout>());
        container->addView(label);
        inflate(Centered{container});
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(testing::AtLeast(1));
        label->setText("ololo");
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        // should not cause invalidation events
        EXPECT_CALL(*container, markPixelDataInvalid).Times(0);
        label->setText("ololo");
        AUI_REPEAT(10) { uitest::frame(); }
    }
}

TEST_F(UIRepaintTest, ContainerRepaintTest4) {
    testing::InSequence s;
    auto label = _new<ALabel>("test");
    auto container = _new<ViewContainerMock>();

    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(BASE_REPAINT_COUNT);

        container->setLayout(_new<AHorizontalLayout>());
        container->addView(label);
        inflate(Centered{container});
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(testing::AtLeast(1));
        label->setText("ololo");
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        // adding extra view should trigger an update
        EXPECT_CALL(*container, markPixelDataInvalid).Times(2);
        container->addView(Label { "another label" });
        AUI_REPEAT(10) { uitest::frame(); }
    }
}

TEST_F(UIRepaintTest, ContainerRepaintTest5) {
    testing::InSequence s;
    auto label = _new<ALabel>("test");
    auto container = _new<ViewContainerMock>();

    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(BASE_REPAINT_COUNT);

        container->setLayout(_new<AHorizontalLayout>());
        container->addView(label);
        inflate(Centered{container});
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        EXPECT_CALL(*container, markPixelDataInvalid).Times(testing::AtLeast(1));
        label->setText("ololo");
        AUI_REPEAT(10) { uitest::frame(); }
    }
    {
        testing::InSequence s;
        // random layout update request should not trigger repaint event
        EXPECT_CALL(*container, markPixelDataInvalid).Times(0);
        label->markMinContentSizeInvalid();
        AUI_REPEAT(10) { uitest::frame(); }
    }
}
