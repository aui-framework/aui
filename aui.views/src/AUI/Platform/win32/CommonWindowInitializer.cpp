//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonWindowInitializer.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Util/AError.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AWindow* window = reinterpret_cast<AWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    try {
        if (window)
            return window->winProc(hwnd, uMsg, wParam, lParam);
    } catch (const AException& e) {
        AError::handle(e);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void
CommonWindowInitializer::initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                                          AWindow* parent) {

    window.mParentWindow = parent;

    // CREATE WINDOW
    WNDCLASSEX winClass;


    ARandom r;
    for (;;) {
        window.mWindowClass = "AUI-" + AString::number(r.nextInt());
        winClass.lpszClassName = window.mWindowClass.c_str();
        winClass.cbSize = sizeof(WNDCLASSEX);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = WindowProc;
        winClass.hInstance = window.mInst;
        //winClass.hIcon = LoadIcon(mInst, (LPCTSTR)101);
        //winClass.hIconSm = LoadIcon(mInst, (LPCTSTR)101);
        winClass.hIcon = 0;
        winClass.hIconSm = 0;
        winClass.hbrBackground = nullptr;
        winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        winClass.lpszMenuName = window.mWindowClass.c_str();
        winClass.cbClsExtra = 0;
        winClass.cbWndExtra = 0;
        if (RegisterClassEx(&winClass)) {
            break;
        }
    }

    DWORD style = WS_OVERLAPPEDWINDOW;

    window.mHandle = CreateWindowEx(WS_EX_DLGMODALFRAME, window.mWindowClass.c_str(), name.c_str(), style,
                             GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
                             GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2, width, height,
                             parent != nullptr ? parent->mHandle : nullptr, nullptr, window.mInst, nullptr);

    window.mDC = GetDC(window.mHandle);

    SetWindowLongPtr(window.mHandle, GWLP_USERDATA, reinterpret_cast<long long int>(&window));

    // used for ACustomWindow
    window.winProc(window.mHandle, WM_CREATE, 0, 0);

    if ((ws & WindowStyle::MODAL) && parent) {
        EnableWindow(parent->mHandle, false);
    }
}

void CommonWindowInitializer::destroyNativeWindow(AWindow& window) {
    wglMakeCurrent(window.mDC, nullptr);
    ReleaseDC(window.mHandle, window.mDC);

    DestroyWindow(window.mHandle);
    UnregisterClass(window.mWindowClass.c_str(), window.mInst);
}
