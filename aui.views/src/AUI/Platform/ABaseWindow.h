//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AWindowManager.h"
#include "AOverlappingSurface.h"
#include <optional>



class API_AUI_VIEWS ABaseWindow: public AViewContainer {
    friend class SoftwareRenderer;
    friend class UITestCaseScope;
    friend class IRenderingContext::Init;
private:
    _weak<AView> mFocusedView;
    _weak<AView> mProfiledView;


    glm::ivec2 mMousePos;
    ASet<_<AOverlappingSurface>> mOverlappingSurfaces;

protected:
    float mDpiRatio = 1.f;
    bool mIsFocused = true;
    _unique<IRenderingContext> mRenderingContext;

    static ABaseWindow*& currentWindowStorage();

    /**
     * @see ABaseWindow::createOverlappingSurface
     */
    virtual _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) = 0;
    virtual void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) = 0;

    virtual void createDevtoolsWindow();

    static _unique<AWindowManager>& getWindowManagerImpl();

public:

    ABaseWindow();

    virtual ~ABaseWindow();
    static AWindowManager& getWindowManager() {
        return *getWindowManagerImpl();
    }
    template<typename WindowManager, typename... Args>
    static void setWindowManager(Args&&... args) {
        destroyWindowManager(); // destroys previous window manager so IEventLoop::Handle sets window manager to the
                                // previous one BEFORE the new window manager is set
        getWindowManagerImpl() = std::make_unique<WindowManager>(std::forward<Args>(args)...);
    }
    static void destroyWindowManager() {
        getWindowManagerImpl() = nullptr;
    }

    const _unique<IRenderingContext>& getRenderingContext() const {
        return mRenderingContext;
    }

    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    void setProfiledView(const _<AView>& view) {
        mProfiledView = view;
    }

    void setFocusedView(const _<AView>& view);
    void focusNextView();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    void closeOverlappingSurfacesOnClick();

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
     * Creates a surface used for context menus and combo boxes. Closure of the surface is controlled by <code>closeOnClick</code> param but can be
     * closed manually by calling closeOverlappingSurface.
     * @param position position where does the surface should be created. It's not exact the top left corner of the
     *        surface but a hint (i.e. if the surface does not fit)
     * @param size size
     * @param closeOnClick when true, overlapped surface is automatically closed when mouse clicked. It's usable for
     *        dropdown and context menus.
     * @return a new surface.
     */
    _<AOverlappingSurface> createOverlappingSurface(const glm::ivec2& position,
                                                    const glm::ivec2& size,
                                                    bool closeOnClick = true) {
        return createOverlappingSurface([&](unsigned attempt) -> std::optional<glm::ivec2> {
            switch (attempt) {
                case 0: return position;
                case 1: return glm::clamp(position, {0, 0}, {getSize() - size});
                default: return std::nullopt;
            }
        }, size, closeOnClick);
    }

    /**
     * Create a surface used for context menus and combo boxes. Closure of the surface is controlled by <code>closeOnClick</code> param but can be
     * closed manually by calling closeOverlappingSurface.
     * @param positionFactory a function that return position where does the surface should be created. If the surface
     *        does not fit, the function is called again with previous arg+1.
     * @param size size
     * @param closeOnClick when true, overlapped surface is automatically closed when mouse clicked. It's usable for
     *        dropdown and context menus.
     * @return a new surface.
     */
    _<AOverlappingSurface> createOverlappingSurface(const std::function<std::optional<glm::ivec2>(unsigned)>& positionFactory,
                                                    const glm::ivec2& size,
                                                    bool closeOnClick = true) {
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
        tmp->mWindow = this;
        tmp->mCloseOnClick = closeOnClick;
        mOverlappingSurfaces << tmp;
        return tmp;
    }
    void closeOverlappingSurface(AOverlappingSurface* surface) {
        closeOverlappingSurfaceImpl(surface);
        mOverlappingSurfaces.erase(aui::ptr::fake(surface));
    }

    void onFocusLost() override;

    void render() override;

signals:
    emits<>            dpiChanged;
    emits<glm::ivec2>  mouseMove;
    emits<AInput::Key> keyDown;

};


