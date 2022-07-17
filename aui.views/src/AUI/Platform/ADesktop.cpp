/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "ADesktop.h"

#ifdef __MINGW32__
// Explicitly setting NTDDI version, this is necessary for the MinGW compiler
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif


#if AUI_PLATFORM_WIN

#include "ADesktop.h"
#include "ACursor.h"
#include "AWindow.h"
#include <windows.h>
#include <shlobj.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Thread/ACutoffSignal.h>

glm::ivec2 ADesktop::getMousePosition() {
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}

void ADesktop::setMousePos(const glm::ivec2& pos) {
    SetCursorPos(pos.x, pos.y);
}


AFuture<APath> ADesktop::browseForDir(ABaseWindow* parent, const APath& startingLocation) {
    nullsafe(parent)->blockUserInput();
    return async noexcept {
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
        {
            IShellItem* psiFolder = nullptr;
            for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                APath current = i;
                current.replaceAll('/', '\\');
                SHCreateItemFromParsingName(current.data(), nullptr, IID_IShellItem,
                                            reinterpret_cast<void**>(&psiFolder));
            }
            if (psiFolder) {
                pFileOpen->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }


        HWND nativeParentWindow;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = d->nativeHandle();
        } else {
            nativeParentWindow = nullptr;
        }

        hr = pFileOpen->Show(nativeParentWindow);

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

        nullsafe(parent)->getThread()->enqueue([parent, pFileOpen] {
            parent->blockUserInput(false);

            pFileOpen->Release();
            CoUninitialize();
            OleUninitialize();
        });

        return result;
    };
}

AFuture<APath> ADesktop::browseForFile(ABaseWindow* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    nullsafe(parent)->blockUserInput();
    return async noexcept {
        APath result;
        OleInitialize(0);
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        assert(SUCCEEDED(hr));
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        assert(SUCCEEDED(hr));
        AVector<COMDLG_FILTERSPEC> filter;
        AVector<AString> storage;
        filter.reserve(extensions.size());
        storage.reserve(extensions.size() * 2);
        for (auto& ext : extensions) {
            auto extFilter = "*." + ext.extension;
            storage << extFilter;
            storage << ext.name + " (" + extFilter + ")";
            filter << COMDLG_FILTERSPEC{ (storage.end()-1)->c_str(), (storage.end()-2)->c_str() };
        }


        hr = pFileOpen->SetFileTypes(filter.size(), filter.data());
        assert(SUCCEEDED(hr));

        {
            IShellItem* psiFolder = nullptr;
            for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                APath current = i;
                current.replaceAll('/', '\\');
                SHCreateItemFromParsingName(current.data(), nullptr, IID_IShellItem,
                                            reinterpret_cast<void**>(&psiFolder));
            }
            if (psiFolder) {
                pFileOpen->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }

        HWND nativeParentWindow;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = d->nativeHandle();
        } else {
            nativeParentWindow = nullptr;
        }

        hr = pFileOpen->Show(nativeParentWindow);

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

        nullsafe(parent)->getThread()->enqueue([parent, pFileOpen] {
            parent->blockUserInput(false);

            pFileOpen->Release();
            CoUninitialize();
            OleUninitialize();
        });

        return result;
    };
}
void ADesktop::openUrl(const AString& url) {
    ShellExecute(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_NORMAL);
}


#elif AUI_PLATFORM_ANDROID

glm::ivec2 ADesktop::getMousePosition() {
    return {};
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(const APath &startingLocation) {
    return async {
        return APath();
    };
}

AFuture<APath>
ADesktop::browseForFile(const APath &startingLocation, const AVector <FileExtension> &extensions) {
    return async {
        return APath();
    };
}

void ADesktop::openUrl(const AString &url) {

}

#elif AUI_PLATFORM_APPLE

// TODO apple
glm::ivec2 ADesktop::getMousePosition() {
    return glm::ivec2();
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(const APath &startingLocation) {
    return AFuture<APath>();
}

AFuture<APath> ADesktop::browseForFile(const APath &startingLocation, const AVector<FileExtension> &extensions) {
    return AFuture<APath>();
}

void ADesktop::openUrl(const AString &url) {

}

#else

#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>
#include "ADesktop.h"
#include "ACursor.h"
#include "AWindow.h"
#include "CommonRenderingContext.h"

void aui_gtk_init() {
    do_once {
        int argc = 0;
        char c = 0;
        char* pc = &c;
        char** ppc = &pc;

        gtk_init(&argc, &ppc);
    };
}

glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(CommonRenderingContext::ourDisplay, XRootWindow(CommonRenderingContext::ourDisplay, 0), &w, &w, &p.x, &p.y, &unused1, &unused1, &unused2);
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
    auto rootWindow = XRootWindow(CommonRenderingContext::ourDisplay, 0);
    XSelectInput(CommonRenderingContext::ourDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(CommonRenderingContext::ourDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(CommonRenderingContext::ourDisplay);
}

void ADesktop::openUrl(const AString& url) {
    std::string s = "xdg-open ";
    s += url.toStdString();
    system(s.c_str());
}

AFuture<APath> ADesktop::browseForFile(const APath& startingLocation, const AVector<FileExtension>& extensions) {
    return async {
        aui_gtk_init();
        GtkWidget *dialog;
        GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        gint res;

        dialog = gtk_file_chooser_dialog_new ("Open file"_i18n.toStdString().c_str(),
                                              nullptr,
                                              action,
                                              "Cancel"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_CANCEL,
                                              "Open"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_ACCEPT,
                                              nullptr);


        gtk_file_chooser_set_current_folder(reinterpret_cast<GtkFileChooser*>(dialog), startingLocation.toStdString().c_str());

        gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
        gtk_window_activate_focus(GTK_WINDOW(dialog));

        res = gtk_dialog_run (GTK_DIALOG (dialog));
        if (res == GTK_RESPONSE_ACCEPT)
        {
            char *filename;
            GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
            filename = gtk_file_chooser_get_filename (chooser);
            APath f = filename;
            g_free(filename);
            gtk_widget_destroy (dialog);

            while (gtk_events_pending()) {
                gtk_main_iteration();
            }
            return f;
        }

        gtk_widget_destroy (dialog);

        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        return APath{};
    };
}

AFuture<APath> ADesktop::browseForDir(const APath& startingLocation) {
    return async {
        aui_gtk_init();
        GtkWidget *dialog;
        GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        gint res;

        dialog = gtk_file_chooser_dialog_new ("Open folder"_i18n.toStdString().c_str(),
                                              nullptr,
                                              action,
                                              "Cancel"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_CANCEL,
                                              "Open"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_ACCEPT,
                                              nullptr);


        gtk_file_chooser_set_current_folder(reinterpret_cast<GtkFileChooser*>(dialog), startingLocation.toStdString().c_str());

        gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
        gtk_window_activate_focus(GTK_WINDOW(dialog));

        res = gtk_dialog_run (GTK_DIALOG (dialog));
        if (res == GTK_RESPONSE_ACCEPT)
        {
            char *filename;
            GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
            filename = gtk_file_chooser_get_filename (chooser);
            APath f = filename;
            g_free(filename);
            gtk_widget_destroy (dialog);

            while (gtk_events_pending()) {
                gtk_main_iteration();
            }
            return f;
        }

        gtk_widget_destroy (dialog);

        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        return APath{};
    };
}

#endif