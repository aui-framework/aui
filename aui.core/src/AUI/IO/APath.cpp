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

#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include "APath.h"
#include <AUI/Common/AStringVector.h>
#include "AIOException.h"
#include "AFileInputStream.h"
#include "AFileOutputStream.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/Util/ACleanup.h"
#include <AUI/Traits/platform.h>
#include <AUI/Util/kAUI.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <AUI/Platform/ErrorToException.h>

#else
#include <dirent.h>
#include <cstring>
#endif

APath APath::parent() const {
    auto c = ensureNonSlashEnding().rfind('/');
    if (c != NPOS) {
        return APath(substr(0, c));
    }
    return {};
}

APath APath::filename() const {
    auto fs = bytes().rfind('/');
    auto bs = bytes().rfind('\\');
    if (fs == NPOS && bs == NPOS) {
        return *this;
    }
    if (fs == NPOS) {
        fs = bs;
    }
    if (bs == NPOS) {
        bs = fs;
    }
    return APath(bytes().substr(std::max(fs, bs) + 1));
}

APath APath::filenameWithoutExtension() const {
    auto name = filename();
    auto it = name.rfind('.');
    if (it == NPOS) {
        return name;
    }
    return APath(name.substr(0, it));
}

AString APath::extension() const {
    auto it = rfind('.');
    return AString::substr(it + 1);
}

APath APath::file(const AString& fileName) const {
    return ensureSlashEnding() + fileName;
}

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

AString APath::relativelyTo(const APath& dir) const {
    if (isAbsolute() == dir.isAbsolute()) {
        auto f = dir.ensureSlashEnding();
        AUI_ASSERT(startsWith(f));
        return substr(f.length());
    }
    auto meButAbsolute = absolute();
    auto f = dir.absolute().ensureSlashEnding();
    AUI_ASSERT(meButAbsolute.startsWith(f));
    return meButAbsolute.substr(f.length());
}
bool APath::exists() const {
    return stat().st_mode & (S_IFDIR | S_IFREG);
}
bool APath::isRegularFileExists() const {
    return stat().st_mode & S_IFREG;
}

bool APath::isDirectoryExists() const {
    return stat().st_mode & S_IFDIR;
}

const APath& APath::removeFile() const {
#if AUI_PLATFORM_WIN
    if (isRegularFileExists()) {
        AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
        if (!DeleteFile(reinterpret_cast<const wchar_t*>(pathU16.data()))) {
            aui::impl::lastErrorToException("could not remove file " + *this);
        }
    } else if (isDirectoryExists()) {
        AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
        if (!RemoveDirectory(reinterpret_cast<const wchar_t*>(pathU16.data()))) {
            aui::impl::lastErrorToException("could not remove directory " + *this);
        }
    } else {
        throw AFileNotFoundException("could not remove file " + *this + ": not exists");
    }
#else
    if (::remove(toStdString().c_str()) != 0) {
        aui::impl::lastErrorToException("could not remove file " + *this);
    }
#endif
    return *this;
}

const APath& APath::removeFileRecursive() const {
    removeDirContentsRecursive();
    if (exists())
        removeFile();
    return *this;
}

const APath& APath::removeDirContentsRecursive() const {
    if (!isDirectoryExists()) {
        return *this;
    }

    for (auto& l : listDir()) {
        l.removeFileRecursive();
    }

    return *this;
}

ADeque<APath> APath::listDir(AFileListFlags f) const {
    ADeque<APath> list;

#ifdef WIN32
    WIN32_FIND_DATA fd;
    auto wPath = aui::win32::toWchar(file("*"));
    HANDLE dir = FindFirstFile(wPath.c_str(), &fd);

    if (dir == INVALID_HANDLE_VALUE) {
#else
    DIR* dir = opendir(toStdString().c_str());
    if (!dir) {
#endif
        aui::impl::lastErrorToException("could not list " + *this);
        return {};
    }

#ifdef WIN32
    for (bool t = true; t; t = FindNextFile(dir, &fd)) {
        AString filename(reinterpret_cast<char16_t*>(fd.cFileName)); // NOLINT(*-pro-type-reinterpret-cast)
        bool isFile = !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        bool isDirectory = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    for (dirent* i = nullptr; (i = readdir(dir));) {
        const auto* filename = i->d_name;
        bool isFile = i->d_type & DT_REG;
        bool isDirectory = i->d_type & DT_DIR;
#endif

        if (!(f & AFileListFlags::DONT_IGNORE_DOTS)) {
            if ("."_as == filename || ".."_as == filename) {
                continue;
            }
        }
        if ((f & AFileListFlags::DIRS && isDirectory) || (f & AFileListFlags::REGULAR_FILES && isFile)) {
            list << file(APath(filename));
        }
        if (f & AFileListFlags::RECURSIVE && isDirectory) {
            auto childDir = file(APath(filename));
            for (auto& file : childDir.listDir(f)) {
                if (file.startsWith(childDir)) {
                    // absolute path
                    auto p = file.substr(childDir.length());
                    if (p.startsWith("/")) {
                        p = p.substr(1);
                    }
                    list << childDir.file(p);
                } else {
                    list << childDir.file(file);
                }
            }
        }
    }
#ifdef WIN32
    FindClose(dir);
#else
    closedir(dir);
#endif
    return list;
}

APath APath::absolute() const {
    if (AString::empty()) {
        return APath::workingDir();
    }

    // *nix requires file existence but windows doesn't - unifying the behaviour
    if (!exists()) {
        throw AFileNotFoundException("could not find absolute file " +*this);
    }
#ifdef WIN32
    AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
    AByteBuffer bufU16;
    bufU16.resize(0x1000 * sizeof(char16_t));
    if (_wfullpath(reinterpret_cast<wchar_t*>(bufU16.data()), reinterpret_cast<const wchar_t*>(pathU16.data()), bufU16.size() / sizeof(char16_t)) == nullptr) {
        aui::impl::lastErrorToException("could not find absolute file \"" + *this + "\"");
    }
    APath buf(reinterpret_cast<const char16_t*>(bufU16.data()));
    buf.resizeToNullTerminator();
    buf.removeBackSlashes();
    return buf;
#else
    auto rawPath = aui::ptr::manage_unique(realpath(toStdString().c_str(), nullptr), free);
    if (!rawPath) {
        aui::impl::lastErrorToException("could not find absolute file " + *this);
    }
    return rawPath.get();
#endif
}

const APath& APath::makeDir() const {
#ifdef WIN32
    AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
    if (CreateDirectory(reinterpret_cast<const wchar_t*>(pathU16.data()), nullptr)) return *this;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // race condition issue.
        return *this;
    }
#else
    if (::mkdir(toStdString().c_str(), 0755) == 0) {
        return *this;
    }
#endif
    aui::impl::lastErrorToException("could not create directory: {}"_format(*this));
    return *this;
}

const APath& APath::makeDirs() const {
    if (empty()) {
        return *this;
    }
    if (!isDirectoryExists()) {
        parent().makeDirs();
        makeDir();
    }
    return *this;
}
void APath::removeBackSlashes() {
    replaceAll('\\', '/');
}

bool APath::isAbsolute() const {
    if (length() >= 1) {
        if (first() == '/')
            return true;
        if (length() >= 2) {
            auto secondChar = operator[](1);
            if (secondChar == ':') {
                return true;
            }
        }
    }
    return false;
}

size_t APath::fileSize() const {
    return stat().st_size;
}

#if AUI_PLATFORM_WIN
struct _stat64 APath::stat() const {
    struct _stat64 s = {0};
    AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
    _wstat64(reinterpret_cast<const wchar_t*>(pathU16.data()), &s);
    return s;
}
#else
struct stat APath::stat() const {
    struct stat s = {0};
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

APath APath::withoutUppermostFolder() const {
    auto r = AString::find('/');
    if (r == NPOS)
        return *this;
    return APath(substr(r + 1));
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

AVector<APath> APath::find(const AString& filename, const AVector<APath>& locations, APathFinder flags) {
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

time_t APath::fileModifyTime() const {
#if AUI_PLATFORM_WIN
    return stat().st_mtime;
#elif AUI_PLATFORM_APPLE
    return stat().st_mtimespec.tv_sec;
#else
    return stat().st_mtim.tv_sec;
#endif
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

AString APath::systemSlashDirection() const {
#if AUI_PLATFORM_WIN
    AString selfCopy = *this;
    selfCopy.replaceAll('/', '\\');
    return selfCopy;
#else
    return *this;
#endif
}

const APath& APath::touch() const {
    AFileOutputStream fos(*this);
    return *this;
}

const APath& APath::chmod(int newMode) const {
#if AUI_PLATFORM_WIN
    AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
    if (::_wchmod(reinterpret_cast<const wchar_t*>(pathU16.data()), newMode) != 0)
#else
    if (::chmod(toStdString().c_str(), newMode) != 0)
#endif
    {
        aui::impl::lastErrorToException("unable to chmod {}"_format(*this));
    }
    return *this;
}

APath APath::extensionChanged(const AString& newExtension) const {
    auto it = rfind('.');
    if (it == NPOS) {
        return *this + "." + newExtension;
    }
    return substr(0, it) + "." + newExtension;
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

bool APath::isEffectivelyAccessible(AFileAccess flags) const noexcept {
#if AUI_PLATFORM_WIN
    int wflags = 0;
    if (bool(flags & AFileAccess::R)) {
        wflags |= 04;
    }
    if (bool(flags & AFileAccess::W)) {
        wflags |= 02;
    }
    if (wflags == 0) {
        return true;
    }
    AByteBuffer pathU16 = encode(AStringEncoding::UTF16);
    return _waccess(reinterpret_cast<const wchar_t*>(pathU16.data()), wflags) == 0;
#elif AUI_PLATFORM_LINUX
    return euidaccess(toStdString().c_str(), int(flags)) == 0;
#elif AUI_PLATFORM_ANDROID
    return access(toStdString().c_str(), int(flags)) == 0;
#elif AUI_PLATFORM_APPLE
    return access(toStdString().c_str(), int(flags)) == 0;
#elif
#error "unimplemented"
#endif
}
