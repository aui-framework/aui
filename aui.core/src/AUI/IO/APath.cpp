// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <AUI/Traits/platform.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <AUI/Platform/ErrorToException.h>

#else
#include <dirent.h>
#include <cstring>
#endif

APath APath::parent() const {
    auto c = ensureNonSlashEnding().rfind(L'/');
    if (c != NPOS) {
        return substr(0, c);
    }
    return {};
}

APath APath::filename() const {
     auto i = rfind(L'/');
     if (i == NPOS) {
         return *this;
     }
    return substr(i + 1);
}

APath APath::filenameWithoutExtension() const {
    auto name = filename();
    auto it = name.rfind('.');
    if (it == NPOS) {
        return name;
    }
    return name.substr(0, it);
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
        return substr(0, length() - 1);
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
        if (!DeleteFile(c_str())) {
            aui::impl::lastErrorToException("could not remove file " + *this);
        }
    } else if (isDirectoryExists()) {
        if (!RemoveDirectory(c_str())) {
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
    if (isDirectoryExists()) {
        for (auto& l : listDir()) {
            l.removeFileRecursive();
        }
    }
    if (exists())
        removeFile();
    return *this;
}

ADeque<APath> APath::listDir(AFileListFlags f) const {
    ADeque<APath> list;

#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE dir = FindFirstFile(file("*").c_str(), &fd);

    if (dir == INVALID_HANDLE_VALUE)
#else
    DIR* dir = opendir(toStdString().c_str());
    if (!dir)
#endif
        aui::impl::lastErrorToException("could not list " + *this);

#ifdef WIN32
    for (bool t = true; t; t = FindNextFile(dir, &fd)) {
        auto& filename = fd.cFileName;
        bool isFile = !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        bool isDirectory = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    for (dirent* i; (i = readdir(dir));) {
        auto& filename = i->d_name;
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
    wchar_t buf[0x1000];
    if (_wfullpath(buf, c_str(), sizeof(buf) / sizeof(wchar_t)) == nullptr) {
        aui::impl::lastErrorToException("could not find absolute file \"" + *this + "\"");
    }

    return APath(buf);
#else
    auto rawPath = aui::ptr::make_unique_with_deleter(realpath(toStdString().c_str(), nullptr), free);
    if (!rawPath) {
        aui::impl::lastErrorToException("could not find absolute file " + *this);
    }
    return rawPath.get();
#endif
}

const APath& APath::makeDir() const {
#ifdef WIN32
    if (CreateDirectory(c_str(), nullptr)) return *this;
#else
    if (::mkdir(toStdString().c_str(), 0755) == 0) {
        return *this;
    }
#endif
    aui::impl::lastErrorToException("could not create directory: {}"_format(*this));
    return *this;
}

const APath& APath::makeDirs() const {
    if (!empty()) {
        if (!isDirectoryExists()) {
            parent().makeDirs();
            makeDir();
        }
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
    _wstat64(c_str(), &s);
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
    AFileOutputStream(destination) << AFileInputStream(source);
}

#if AUI_PLATFORM_WIN
#include <shlobj.h>

APath APath::getDefaultPath(APath::DefaultPath path) {
    APath result;
    result.resize(MAX_PATH);
    switch (path) {
        case APPDATA:
            SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, result.data());
            break;

        case TEMP:
            GetTempPath(result.length(), result.data());
            break;

        case HOME:
            SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, SHGFP_TYPE_DEFAULT, result.data());
            break;

        default:
            AUI_ASSERT(0);
    }
    result.resizeToNullTerminator();
    result.removeBackSlashes();
    return result;
}


APath APath::withoutUppermostFolder() const {
    auto r = AString::find('/');
    if (r == NPOS)
        return *this;
    return substr(r + 1);
}

APath APath::workingDir() {
    APath p;
    p.resize(0x800);
    p.resize(GetCurrentDirectory(0x800, p.data()));
    return p;
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

APath APath::workingDir() {
    auto cwd = aui::ptr::make_unique_with_deleter(getcwd(nullptr, 0), free);
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
        case TEMP:
            return APath::workingDir() / "__aui_tmp";
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
#if AUI_PLATFORM_WIN
    if (MoveFile(source.c_str(), destination.c_str()) == 0) {
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
    if (::_wchmod(c_str(), newMode) != 0)
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
