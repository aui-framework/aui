//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>

class API_AUI_VIEWS ABaseWindow: public AViewContainer {

private:
    _weak<AView> mFocusedView;

protected:
    float mDpiRatio = 1.f;
    bool mIsFocused = true;

    static ABaseWindow*& currentWindowStorage();

public:
    ABaseWindow();

    virtual ~ABaseWindow() = default;

    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    emits<> dpiChanged;

    void setFocusedView(const _<AView>& view);
    void focusNextView();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    bool isFocused() const {
        return mIsFocused;
    }

    emits<AInput::Key> keyDown;

    void onKeyDown(AInput::Key key) override;

    virtual void flagRedraw();
};


