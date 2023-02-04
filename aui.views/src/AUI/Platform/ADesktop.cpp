﻿// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "ADesktop.h"
#include <AUI/Util/ARaiiHelper.h>

#ifdef __MINGW32__
// Explicitly setting NTDDI version, this is necessary for the MinGW compiler
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif


#if AUI_PLATFORM_WIN

#include "AWindow.h"
#include "ADesktop.h"
#include "ACursor.h"
#include "AUI/Platform/win32/Ole.h"
#include "AUI/Util/AImageDrawable.h"
#include "AUI/Platform/win32/Win32Util.h"
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
    AUI_NULLSAFE(parent)->blockUserInput();
    return async noexcept {
        APath result;
        Ole::inst();
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        assert(SUCCEEDED(hr));


        ARaiiHelper d = [&] {
            AUI_NULLSAFE(parent)->getThread()->enqueue([parent, pFileOpen] {
                parent->blockUserInput(false);

                pFileOpen->Release();
            });
        };

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

        return result;
    };
}

AFuture<APath> ADesktop::browseForFile(ABaseWindow* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    AUI_NULLSAFE(parent)->blockUserInput();
    return async noexcept {
        APath result;
        Ole::inst();
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));


        ARaiiHelper d = [&] {
            AUI_NULLSAFE(parent)->getThread()->enqueue([parent, pFileOpen] {
                parent->blockUserInput(false);

                pFileOpen->Release();
            });
        };

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


        return result;
    };
}
void ADesktop::openUrl(const AString& url) {
    ShellExecute(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_NORMAL);
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    SHFILEINFO info;
    if (SUCCEEDED(SHGetFileInfo(file.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))) {

        ARaiiHelper destroyer = [&]{ DestroyIcon(info.hIcon); };
        return _new<AImageDrawable>(_new<AImage>(aui::win32::iconToImage(info.hIcon)));
    }
    return nullptr;
}

#elif AUI_PLATFORM_ANDROID

glm::ivec2 ADesktop::getMousePosition() {
    return {};
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(ABaseWindow* parent,
                                      const APath& startingLocation)  {
    return async {
        return APath();
    };
}

AFuture<APath>
ADesktop::browseForFile(ABaseWindow* parent,
                        const APath& startingLocation,
                        const AVector<FileExtension>& extensions) {
    return async {
        return APath();
    };
}

void ADesktop::openUrl(const AString &url) {

}


_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}

#elif AUI_PLATFORM_APPLE

#include "AWindow.h"
// TODO apple
glm::ivec2 ADesktop::getMousePosition() {
    return glm::ivec2();
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {

}

AFuture<APath> ADesktop::browseForDir(ABaseWindow* parent, const APath &startingLocation) {
    return AFuture<APath>();
}

AFuture<APath> ADesktop::browseForFile(ABaseWindow* parent, const APath &startingLocation, const AVector<FileExtension> &extensions) {
    return AFuture<APath>();
}

void ADesktop::openUrl(const AString &url) {

}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}
#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>
#include "ADesktop.h"
#include "ACursor.h"
#include "AWindow.h"
#include "CommonRenderingContext.h"
#include "AWindow.h"
#undef signals
#include <gtk/gtk.h>

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

AFuture<APath> ADesktop::browseForFile(ABaseWindow* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    parent->blockUserInput();
    return async {
        ARaiiHelper windowUnblocker = [&] {
            parent->getThread()->enqueue([parent] {
                parent->blockUserInput(false);
            });
        };
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

AFuture<APath> ADesktop::browseForDir(ABaseWindow* parent, const APath& startingLocation) {
    parent->blockUserInput();
    return async  {
        ARaiiHelper windowUnblocker = [&] {
            parent->getThread()->enqueue([parent] {
                parent->blockUserInput(false);
            });
        };
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

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}
#endif