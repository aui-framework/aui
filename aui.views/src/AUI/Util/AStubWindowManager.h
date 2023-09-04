#pragma once

/**
 * @brief Window manager used to stub the default window manager.
 * @ingroup views
 * @details
 * Useful for command-line only use-cases where the actual windows are not viable (i.e. tests).
 * Rendering is switched to software when.
 *
 * @code{cpp}
 * window = _new<AWindow>("");
 * window->show();
 * ...
 * auto screenshot = AStubWindowManager::makeScreenshot(window);
 * @endcode
 */
class API_AUI_VIEWS AStubWindowManager: public AWindowManager {
public:
    void initNativeWindow(const IRenderingContext::Init& init) override;
    static AImage makeScreenshot(aui::no_escape<AWindow> window);
    static void drawFrame();
};
