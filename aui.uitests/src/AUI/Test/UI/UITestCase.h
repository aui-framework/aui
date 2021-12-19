//
// Created by Alex2772 on 12/18/2021.
//

#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/Software/SoftwareRenderer.h>
#include <boost/test/tree/observer.hpp>
#include <boost/test/framework.hpp>


class UITestRenderingContext: public SoftwareRenderingContext {
public:
    void init(const Init& init) override {
        IRenderingContext::init(init);
    }

    ~UITestRenderingContext() override = default;

    void destroyNativeWindow(AWindow& window) override {

    }

    void beginPaint(AWindow& window) override {
        std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
    }

    void endPaint(AWindow& window) override {

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


class UITestCaseScope: public boost::unit_test::test_observer {
public:
    UITestCaseScope() {
        boost::unit_test::framework::register_observer(*this);
        Render::setRenderer(std::make_unique<SoftwareRenderer>());
        AWindow::setWindowManager(std::make_unique<UITestWindowManager>());
        ABaseWindow::currentWindowStorage() = nullptr;
    }
    ~UITestCaseScope() {
        boost::unit_test::framework::deregister_observer(*this);
    }

    API_AUI_UITESTS void test_unit_aborted(const boost::unit_test::test_unit& unit) override;
};

#define UI_TEST_CASE(name) void name ## _wrap(); BOOST_AUTO_TEST_CASE(name) { UITestCaseScope fixture; name ## _wrap(); }  void name ## _wrap()