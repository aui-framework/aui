//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AWindowManager.h"
#include <optional>

class API_AUI_VIEWS ABaseWindow: public AViewContainer {
private:
    _weak<AView> mFocusedView;
    glm::ivec2 mMousePos;
    ASet<AViewContainer*> mOverlappingSurfaces;

protected:
    float mDpiRatio = 1.f;
    bool mIsFocused = true;

    static ABaseWindow*& currentWindowStorage();

    static void checkForStencilBits();

    /**
     * @see ABaseWindow::createOverlappingSurface
     */
    virtual _<AViewContainer> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) = 0;
    virtual void closeOverlappingSurfaceImpl(AViewContainer* surface) = 0;

    virtual void createDevtoolsWindow();

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

    void closeAllOverlappingSurfaces();

    bool isFocused() const {
        return mIsFocused;
    }

    [[nodiscard]]
    const glm::ivec2& getMousePos() const {
        return mMousePos;
    }


    void onKeyDown(AInput::Key key) override;

    virtual void flagRedraw();
    virtual void flagUpdateLayout();

    void onKeyUp(AInput::Key key) override;

    void onCharEntered(wchar_t c) override;

    void makeCurrent() {
        currentWindowStorage() = this;
    }

    /**
     * Creates a surface used for context menus and combo boxes. Closure of the surface is managed by window but can be
     * closed manually by calling closeOverlappingSurface.
     * @param position position where does the surface should be created. It's not exact the top left corner of the
     *        surface but a hint (i.e. if the surface does not fit)
     * @param size size
     * @return a new surface.
     */
    _<AViewContainer> createOverlappingSurface(const glm::ivec2& position, const glm::ivec2& size) {
        return createOverlappingSurface([&](unsigned attempt) -> std::optional<glm::ivec2> {
            switch (attempt) {
                case 0: return position;
                case 1: return glm::clamp(position, {0, 0}, {getSize() - size});
                default: return std::nullopt;
            }
        }, size);
    }

    /**
     * Create a surface used for context menus and combo boxes. Closure of the surface is managed by window but can be
     * closed manually by calling closeOverlappingSurface.
     * @param positionFactory a function that return position where does the surface should be created. If the surface
     *        does not fit, the function is called again with previous arg+1.
     * @param size size
     * @return a new surface.
     */
    _<AViewContainer> createOverlappingSurface(const std::function<std::optional<glm::ivec2>(unsigned)>& positionFactory, const glm::ivec2& size) {
        glm::ivec2 position = {0, 0};
        auto maxPos = getSize() - size;
        for (unsigned index = 0; ; ++index) {
            auto optionalPosition = positionFactory(index);
            if (optionalPosition.has_value()) {
                position = *optionalPosition;

                if (position.x >= 0 && position.y >= 0 && glm::all(glm::lessThan(position, maxPos))) {
                    break;
                }
            } else {
                break;
            }
        }

        auto tmp = createOverlappingSurfaceImpl(position, size);
        mOverlappingSurfaces << tmp.get();
        return tmp;
    }
    void closeOverlappingSurface(AViewContainer* surface) {
        mOverlappingSurfaces.erase(surface);
        closeOverlappingSurfaceImpl(surface);
    }

    void onFocusLost() override;

signals:
    emits<>            dpiChanged;
    emits<glm::ivec2>  mouseMove;
    emits<AInput::Key> keyDown;

};


