#ifdef __MINGW32__
// Explicitly setting NTDDI version, this is necessary for the MinGW compiler
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#include "ADesktop.h"
#include "ACursor.h"
#include "AWindow.h"

#if defined(_WIN32)

#include <windows.h>
#include <shlobj.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Logging/ALogger.h>

glm::ivec2 ADesktop::getMousePosition() {
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}

void ADesktop::setMousePos(const glm::ivec2& pos) {
    SetCursorPos(pos.x, pos.y);
}

_<AFuture<APath>> ADesktop::browseForFolder(const APath& startingLocation) {
    return async {
        APath result;
        OleInitialize(0);
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        assert(SUCCEEDED(hr));
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        assert(SUCCEEDED(hr));

        pFileOpen->SetOptions(FOS_PICKFOLDERS);
        IShellItem* psiFolder = nullptr;
        for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
            APath current = i;
            current.replaceAll('/', '\\');
            SHCreateItemFromParsingName(current.data(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psiFolder));
        }
        pFileOpen->SetFolder(psiFolder);
        psiFolder->Release();

        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // Display the file name to the user.
                if (SUCCEEDED(hr))
                {
                    result = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
        CoUninitialize();
        OleUninitialize();
        return result;
    };
}

#elif defined(ANDROID)
glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
}
#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
extern Display* gDisplay;
glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(gDisplay, XRootWindow(gDisplay, 0), &w, &w, &p.x, &p.y, &unused1, &unused1, &unused2);
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
    auto rootWindow = XRootWindow(gDisplay, 0);
    XSelectInput(gDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(gDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(gDisplay);
}
#endif