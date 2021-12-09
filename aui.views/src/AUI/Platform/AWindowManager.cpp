#include "AWindowManager.h"
#include "OpenGLWindowInitializer.h"


void AWindowManager::initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                                      AWindow* parent) {
    auto call = [&] {
        getWindowInitializer()->initNativeWindow(window, name, width, height, ws, parent);
    };
    try {
        call();
    } catch (const AException& e) {
    }
}

const _unique<IWindowInitializer>& AWindowManager::getWindowInitializer() {
    if (!mWindowInitializer) {
        mWindowInitializer = std::make_unique<OpenGLWindowInitializer>();
    }
    return mWindowInitializer;
}
