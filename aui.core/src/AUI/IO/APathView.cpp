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
// Created by nelonn on 9/12/25.
//

#include "APathView.h"

#include <sys/stat.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Platform/ErrorToException.h>
#include <AUI/Traits/platform.h>

#if AUI_PLATFORM_WIN
#include <Windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#endif

APath APathView::ensureSlashEnding() const {
    if (endsWith("/")) {
        return *this;
    }
    return string() + "/";
}

APathView APathView::ensureNonSlashEnding() const {
    if (endsWith("/")) {
        return substr(0, length() - 1);
    }
    return *this;
}

#if AUI_PLATFORM_WIN
struct _stat64 APathView::stat() const {
    const AStringView& self = *this;
    struct _stat64 s = {0};
    std::wstring pathU16 = aui::win32::toWchar(self);
    _wstat64(pathU16.c_str(), &s);
    return s;
}
#else
struct stat APathView::stat() const {
    const AStringView& self = *this;
    AString nt_str(self);
    struct stat s = {};
    ::stat(nt_str.c_str(), &s);
    return s;
}
#endif

APathView::APathView(std::string_view str) : super(str) {}

APathView::APathView(AStringView str) : super(str.bytes()) {}

APath APathView::parent() const {
    auto c = ensureNonSlashEnding().rfind('/');
    if (c != NPOS) {
        return substr(0, c);
    }
    return {};
}

APathView APathView::filename() const {
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
    return APathView(bytes().substr(std::max(fs, bs) + 1));
}

APathView APathView::filenameWithoutExtension() const {
    auto name = filename();
    auto it = name.rfind('.');
    if (it == NPOS) {
        return name;
    }
    return name.substr(0, it);
}

AStringView APathView::extension() const {
    auto it = rfind('.');
    return substr(it + 1);
}

APath APathView::file(AStringView fileName) const {
    return ensureSlashEnding() + fileName;
}

AString APathView::relativelyTo(APathView dir) const {
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
bool APathView::exists() const {
    return stat().st_mode & (S_IFDIR | S_IFREG);
}
bool APathView::isRegularFileExists() const {
    return stat().st_mode & S_IFREG;
}

bool APathView::isDirectoryExists() const {
    return stat().st_mode & S_IFDIR;
}

APathView APathView::removeFile() const {
#if AUI_PLATFORM_WIN
    if (isRegularFileExists()) {
        auto pathU16 = aui::win32::toWchar(string());
        if (!DeleteFile(pathU16.c_str())) {
            aui::impl::lastErrorToException("could not remove file " + string());
        }
    } else if (isDirectoryExists()) {
        auto pathU16 = aui::win32::toWchar(string());
        if (!RemoveDirectory(pathU16.c_str())) {
            aui::impl::lastErrorToException("could not remove directory " + string());
        }
    } else {
        throw AFileNotFoundException("could not remove file " + string() + ": not exists");
    }
#else
    if (::remove(toStdString().c_str()) != 0) {
        aui::impl::lastErrorToException("could not remove file " + string());
    }
#endif
    return *this;
}

APathView APathView::removeFileRecursive() const {
    removeDirContentsRecursive();
    if (exists()) {
        removeFile();
    }
    return *this;
}

APathView APathView::removeDirContentsRecursive() const {
    if (!isDirectoryExists()) {
        return *this;
    }

    for (auto& l : listDir()) {
        l.removeFileRecursive();
    }

    return *this;
}

ADeque<APath> APathView::listDir(AFileListFlags f) const {
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
        aui::impl::lastErrorToException("could not list " + string());
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
        bool isFile = false;
        bool isDirectory = false;
        if (i->d_type == DT_UNKNOWN) {
            struct stat st;
            if (lstat(file(i->d_name).c_str(), &st) == 0) {
                isFile = S_ISREG(st.st_mode);
                isDirectory = S_ISDIR(st.st_mode);
            }
        } else {
            isFile = i->d_type & DT_REG;
            isDirectory = i->d_type & DT_DIR;
        }
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

APath APathView::absolute() const {
    if (super::empty()) {
        return APath::workingDir();
    }

    // *nix requires file existence but windows doesn't - unifying the behaviour
    if (!exists()) {
        throw AFileNotFoundException("could not find absolute file " + string());
    }
#ifdef WIN32
    auto pathU16 = aui::win32::toWchar(string());
    AByteBuffer bufU16;
    bufU16.resize(0x1000 * sizeof(char16_t));
    if (_wfullpath(reinterpret_cast<wchar_t*>(bufU16.data()), pathU16.c_str(), bufU16.size() / sizeof(char16_t)) == nullptr) {
        aui::impl::lastErrorToException("could not find absolute file \"" + string() + "\"");
    }
    APath buf(reinterpret_cast<const char16_t*>(bufU16.data()));
    buf.resizeToNullTerminator();
    buf.removeBackSlashes();
    return buf;
#else
    auto rawPath = aui::ptr::manage_unique(realpath(toStdString().c_str(), nullptr), free);
    if (!rawPath) {
        aui::impl::lastErrorToException("could not find absolute file " + string());
    }
    return rawPath.get();
#endif
}

APathView APathView::makeDir() const {
#ifdef WIN32
    auto pathU16 = aui::win32::toWchar(string());
    if (CreateDirectory(pathU16.c_str(), nullptr)) return *this;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // race condition issue.
        return *this;
    }
#else
    if (::mkdir(toStdString().c_str(), 0755) == 0) {
        return *this;
    }
#endif
    aui::impl::lastErrorToException("could not create directory: {}"_format(string()));
    return *this;
}

APathView APathView::makeDirs() const {
    if (empty()) {
        return *this;
    }
    if (!isDirectoryExists()) {
        parent().makeDirs();
        makeDir();
    }
    return *this;
}

bool APathView::isAbsolute() const {
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

size_t APathView::fileSize() const {
    return stat().st_size;
}

APath APathView::withoutUppermostFolder() const {
    auto r = super::find('/');
    if (r == NPOS)
        return *this;
    return substr(r + 1);
}

time_t APathView::fileModifyTime() const {
#if AUI_PLATFORM_WIN
    return stat().st_mtime;
#elif AUI_PLATFORM_APPLE
    return stat().st_mtimespec.tv_sec;
#else
    return stat().st_mtim.tv_sec;
#endif
}

AString APathView::systemSlashDirection() const {
#if AUI_PLATFORM_WIN
    AString selfCopy = string();
    selfCopy.replaceAll('/', '\\');
    return selfCopy;
#else
    return string();
#endif
}

APathView::APathView() {}
APathView::APathView(const APathView& other) = default;
APathView::APathView(APathView&& other) noexcept = default;
APathView::APathView(const APath& path) : super(path.string()) {}

const APathView& APathView::touch() const {
    AFileOutputStream fos(toStdString());
    return *this;
}

APathView APathView::chmod(int newMode) const {
#if AUI_PLATFORM_WIN
    auto pathU16 = aui::win32::toWchar(string());
    if (::_wchmod(pathU16.c_str(), newMode) != 0)
#else
    if (::chmod(toStdString().c_str(), newMode) != 0)
#endif
    {
        aui::impl::lastErrorToException("unable to chmod {}"_format(string()));
    }
    return *this;
}

APath APathView::extensionChanged(AStringView newExtension) const {
    auto it = rfind('.');
    if (it == NPOS) {
        return string() + "." + newExtension;
    }
    return substr(0, it) + "." + newExtension;
}

bool APathView::isEffectivelyAccessible(AFileAccess flags) const noexcept {
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
    auto pathU16 = aui::win32::toWchar(string());
    return _waccess(pathU16.c_str(), wflags) == 0;
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
