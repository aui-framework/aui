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

//
// Created by Alex2772 on 12/18/2021.
//

#pragma once


#include <gtest/gtest.h>

#include <AUI/Platform/AWindow.h>
#include <AUI/Software/SoftwareRenderer.h>


class UITestRenderingContext: public SoftwareRenderingContext {
public:
    void init(const Init& init) override {
        IRenderingContext::init(init);
    }

    ~UITestRenderingContext() override = default;

    void destroyNativeWindow(ABaseWindow& window) override {

    }

    void beginPaint(ABaseWindow& window) override {
        std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
    }

    void endPaint(ABaseWindow& window) override {

    }

};

class UITestWindowManager: public AWindowManager {
public:
    void initNativeWindow(const IRenderingContext::Init& init) override {
        auto context = std::make_unique<UITestRenderingContext>();
        context->init(init);
        init.setRenderingContext(std::move(context));
    }
};

namespace testing {
    class API_AUI_UITESTS UITest : public testing::Test {
    public:
    protected:
        void SetUp() override;

        void TearDown() override;

    public:
    };
}