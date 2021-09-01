//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AWindowManager.h"

class API_AUI_VIEWS ABaseWindow: public AViewContainer {

private:
    _weak<AView> mFocusedView;
    glm::ivec2 mMousePos;

protected:
    float mDpiRatio = 1.f;
    bool mIsFocused = true;

    static ABaseWindow*& currentWindowStorage();

    static void checkForStencilBits();

public:
    ABaseWindow();

    virtual ~ABaseWindow() = default;

    AWindowManager& getWindowManager() const;

    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    void setFocusedView(const _<AView>& view);
    void focusNextView();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    bool isFocused() const {
        return mIsFocused;
    }

    [[nodiscard]]
    const glm::ivec2& getMousePos() const {
        return mMousePos;
    }

    emits<AInput::Key> keyDown;

    void onKeyDown(AInput::Key key) override;

    virtual void flagRedraw();
    virtual void flagUpdateLayout();

    void onKeyUp(AInput::Key key) override;

    void onCharEntered(wchar_t c) override;

    void makeCurrent() {
        currentWindowStorage() = this;
    }

signals:
    emits<> dpiChanged;
};


