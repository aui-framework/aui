/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2772 on 9/10/20.
//

#include "APath.h"

#include <sys/stat.h>
#include <cassert>
#include <AUI/Common/AStringVector.h>
#include <AUI/IO/AIOException.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Platform/ErrorToException.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/ACleanup.h>
#include <AUI/Traits/platform.h>
#include <AUI/Util/kAUI.h>

#ifdef WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <cstring>
#endif

APath APath::ensureSlashEnding() const {
    if (endsWith("/")) {
        return *this;
    }
    return (*this) + "/";
}

APath APath::ensureNonSlashEnding() const {
    if (endsWith("/")) {
        return APath(substr(0, length() - 1));
    }
    return *this;
}

void APath::removeBackSlashes() {
    replaceAll('\\', '/');
}

#if AUI_PLATFORM_WIN
struct _stat64 APath::stat() const {
    struct _stat64 s = {0};
    std::wstring pathU16 = aui::win32::toWchar(*this);
    _wstat64(pathU16.c_str(), &s);
    return s;
}
#else
struct stat APath::stat() const {
    struct stat s = {};
    ::stat(toStdString().c_str(), &s);
    return s;
}
#endif


void APath::copy(const APath& source, const APath& destination) {
    if (!source.isRegularFileExists()) {
        throw AFileNotFoundException("APath::copy: regular file (source) does not exist: {}"_format(source));
    }
    AFileOutputStream(destination) << AFileInputStream(source);
}

#if AUI_PLATFORM_WIN
#include <shlobj.h>

APath APath::getDefaultPath(APath::DefaultPath path) {
    AByteBuffer resultU16;
    resultU16.resize(MAX_PATH * sizeof(wchar_t));
    switch (path) {
        case APPDATA:
            SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, reinterpret_cast<wchar_t*>(resultU16.data()));
            break;

        case TEMP:
            GetTempPath(MAX_PATH, reinterpret_cast<wchar_t*>(resultU16.data()));
            break;

        case HOME:
            SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, SHGFP_TYPE_DEFAULT, reinterpret_cast<wchar_t*>(resultU16.data()));
            break;

        default:
            AUI_ASSERT(0);
    }
    APath result(reinterpret_cast<const char16_t*>(resultU16.data()));
    result.resizeToNullTerminator();
    result.removeBackSlashes();
    return result;
}

APath APath::workingDir() {
    AByteBuffer resultU16;
    resultU16.resize(0x800 * sizeof(char16_t));
    resultU16.resize(GetCurrentDirectory(resultU16.size() / sizeof(char16_t), reinterpret_cast<wchar_t*>(resultU16.data())));
    APath p(reinterpret_cast<const char16_t*>(resultU16.data()));
    p.removeBackSlashes();
    return p;
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

APath APath::workingDir() {
    auto cwd = aui::ptr::manage_unique(getcwd(nullptr, 0), free);
    if (!cwd) {
        aui::impl::lastErrorToException("could not find workingDir");
    }
    return cwd.get();
}

APath APath::getDefaultPath(APath::DefaultPath path) {
    switch (path) {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
        case APPDATA:
            return APath::workingDir() / "__aui_appdata";
        case TEMP: {
            auto dir = APath::workingDir() / "__aui_tmp";
            AUI_DO_ONCE {
                // we have to clean up it by ourselves.
                dir.removeFileRecursive();
            }
            return dir;
        }
#else
        case APPDATA:
            return getDefaultPath(HOME) / ".local/share";

        case HOME:
            if (auto home = getenv("HOME")) {
                return home;
            }
            return getpwuid(getuid())->pw_dir;

        case TEMP:
            return "/tmp";
        default:
            AUI_ASSERT(0);
#endif
    }
    return {};
}
#endif

AVector<APath> APath::find(AStringView filename, const AVector<APath>& locations, APathFinder flags) {
    AVector<APath> result;
    auto doReturn = [&] {
        return !!(flags & APathFinder::SINGLE) && !result.empty();
    };
    auto locateImpl = [&](const AVector<AString>& container) {
        auto c = [&](auto& c, const AVector<AString>& container) mutable {
            for (const APath& pathEntry : container) {
                auto fullPath = pathEntry / filename;
                if (fullPath.isRegularFileExists()) {
                    result << fullPath;
                    if (doReturn()) {
                        return;
                    }
                }

                if (!!(flags & APathFinder::RECURSIVE)) {
                    if (pathEntry.isDirectoryExists()) {
                        try {
                            auto list = pathEntry.listDir();
                            c(c, AVector<AString>{list.begin(), list.end()});
                        } catch (...) {}
                    }
                }
            }

        };
        c(c, container);
    };

    locateImpl((const AVector<AString>&) locations);
    if (doReturn()) {
        return result;
    }

    if (!!(flags & APathFinder::USE_SYSTEM_PATHS)) {
        locateImpl(AString(getenv("PATH")).split(aui::platform::current::path_variable_separator));
    }
    return result;
}

void APath::move(const APath& source, const APath& destination) {
    if (source.isRegularFileExists()) {
        if (destination.isRegularFileExists()) {
            try {
                destination.removeFile();
            } catch (...) {}
        }
    }

#if AUI_PLATFORM_WIN
    auto wSource = aui::win32::toWchar(source);
    auto wDestination = aui::win32::toWchar(destination);
    if (MoveFile(wSource.c_str(), wDestination.c_str()) == 0) {
#else
    if (rename(source.toStdString().c_str(), destination.toStdString().c_str())) {
#endif
        aui::impl::lastErrorToException(R"(could not rename "{}" to "{}")"_format(source, destination));
    }
}

const APath& APath::processTemporaryDir() {
    static APath result = [] {
      APath result = APath::getDefaultPath(TEMP) / "aui-temp-{}"_format(AProcess::self()->getPid());
      while (result.exists()) {
          result += "-";
      }
      result.makeDirs();
      ACleanup::afterEntry([result] {
          try {
              result.removeFileRecursive();
          } catch (...) {
          }
      });

      return result;
    }();
    return result;
}

APath APath::nextRandomTemporary() {
    auto base = processTemporaryDir();
    static std::atomic_uint64_t i = 0;
    for (;;) {
        APath result = base / "randomtemporary{}"_format(i++);
        if (!result.exists()) {
            return result;
        }
    }
}
