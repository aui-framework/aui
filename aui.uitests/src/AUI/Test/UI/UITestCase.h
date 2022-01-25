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