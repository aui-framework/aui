/**
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
#include "AUI/Traits/arrays.h"
#include <AUI/Traits/platform.h>

constexpr std::size_t PATH_BUFFER_SIZE = 0x1000;

#ifdef WIN32
#include <windows.h>
#include <direct.h>

AString error_message() {
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return {}; //No error message has been recorded

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    AString message(messageBuffer);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

#define ERROR_DESCRIPTION + ": " + error_message()
#else
#include <dirent.h>
#define ERROR_DESCRIPTION  + ": " + strerror(errno)
#endif

APath APath::parent() const {
    auto c = ensureNonSlashEnding().rfind(L'/');
    if (c != npos) {
        return substr(0, c);
    }
    return {};
}

AString APath::filename() const {
     auto i = rfind(L'/');
     if (i == npos) {
         return *this;
     }
    return substr(i + 1);
}

AString APath::filenameWithoutExtension() const {
    auto name = filename();
    auto it = name.rfind('.');
    if (it == npos) {
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
        assert(startsWith(f));
        return substr(f.length());
    }
    auto meButAbsolute = absolute();
    auto f = dir.absolute().ensureSlashEnding();
    assert(meButAbsolute.startsWith(f));
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
    if (::_wremove(toUtf16().c_str()) != 0) {
#else
    if (::remove(toStdString().c_str()) != 0) {
#endif
#if AUI_PLATFORM_WIN
        if (RemoveDirectory(toUtf16().c_str()))
            return *this;
#endif
        throw AIOException("could not remove file " + *this ERROR_DESCRIPTION);
    }
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

ADeque<APath> APath::listDir(ListFlags f) const {
    ADeque<APath> list;

#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE dir = FindFirstFile(file("*").toUtf16().c_str(), &fd);

    if (dir == INVALID_HANDLE_VALUE)
#else
    DIR* dir = opendir(toStdString().c_str());
    if (!dir)
#endif
        throw AAccessDeniedException("could not list " + *this ERROR_DESCRIPTION);

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

        if (!(f & ListFlags::DONT_IGNORE_DOTS)) {
            if (std::wstring_view(L".") == filename || std::wstring_view(L"..") == filename) {
                continue;
            }
        }
        if ((f & ListFlags::DIRS && isDirectory) || (f & ListFlags::REGULAR_FILES && isFile)) {
            list << file(APath(filename));
        }
        if (f & ListFlags::RECURSIVE && isDirectory) {
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
    // *nix requires file existence but windows doesn't - unifying the behaviour
    if (!exists()) {
        throw AFileNotFoundException("could not find absolute file " +*this);
    }
#ifdef WIN32
    wchar_t buf[PATH_BUFFER_SIZE];
    if (_wfullpath(buf, toUtf16().c_str(), PATH_BUFFER_SIZE) == nullptr) {
        throw AIOException("could not find absolute file" + *this ERROR_DESCRIPTION);
    }

    return APath(buf);
#else
    char buf[PATH_BUFFER_SIZE];
    if (realpath(toStdString().c_str(), buf) == nullptr) {
        throw AFileNotFoundException("could not find absolute file " + *this ERROR_DESCRIPTION);
    }

    return buf;
#endif
}

const APath& APath::makeDir() const {
#ifdef WIN32
    //                                 VVV - КОЗЛЫ, МЛЯТЬ!
    if (::_wmkdir(toUtf16().c_str()) == 0) {
        auto s = "could not create directory: "_as + absolute() ERROR_DESCRIPTION;
        auto et = GetLastError();
        switch (et) {
            case ERROR_FILE_NOT_FOUND:
                throw AFileNotFoundException(s);
            case ERROR_ACCESS_DENIED:
                throw AAccessDeniedException(s);
            case ERROR_ALREADY_EXISTS:
                break;
            default:
                throw AIOException(s);
        }
    }
#else
    if (::mkdir(toStdString().c_str(), 0755) != 0) {
        throw AIOException("could not create directory: "_as + absolute() ERROR_DESCRIPTION);
    }
#endif
    return *this;
}

const APath& APath::makeDirs() const noexcept {
    try {
        if (!empty()) {
            if (!isDirectoryExists()) {
                parent().makeDirs();
                makeDir();
            }
        }
    } catch (...) {

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
        if (length() >= 2 && AString::operator[](1) == ':') {
            return true;
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
    _wstat64(toUtf16().c_str(), &s);
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
    wchar_t buf[PATH_BUFFER_SIZE];
    switch (path) {
        case APPDATA:
            SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, buf);
            break;
        case TEMP:
            GetTempPath(PATH_BUFFER_SIZE, buf);
            break;
        default:
            assert(0);
    }
    APath result(buf);
    result.removeBackSlashes();
    return result;
}


APath APath::withoutUppermostFolder() const {
    auto r = AString::find('/');
    if (r == npos)
        return *this;
    return substr(r + 1);
}

APath APath::workingDir() {
    wchar_t buf[PATH_BUFFER_SIZE];
    GetCurrentDirectory(PATH_BUFFER_SIZE, buf);
    return APath(buf);
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

APath APath::workingDir() {
    char buf[PATH_BUFFER_SIZE];
    getcwd(buf, sizeof(buf));
    return buf;
}

APath APath::getDefaultPath(APath::DefaultPath path) {
    switch (path) {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
        case APPDATA:
            return APath(".").absolute()["__aui_appdata"];
        case TEMP:
            return APath(".").absolute()["__aui_tmp"];
#else
        case APPDATA:
            return APath(getpwuid(getuid())->pw_dir).file(".local/share");

        case TEMP:
            return "/tmp";
        default:
            assert(0);
#endif
    }
    return {};
}
#endif

AVector<APath> APath::find(const AString& filename, const AVector<APath>& locations, PathFinder flags) {
    AVector<APath> result;
    auto doReturn = [&] {
        return !!(flags & PathFinder::SINGLE) && !result.empty();
    };
    auto locateImpl = [&](const AVector<AString>& container) {
        auto c = [&](auto& c, const AVector<AString>& container) mutable {
            for (const APath& pathEntry : container) {
                auto fullPath = pathEntry[filename];
                if (fullPath.isRegularFileExists()) {
                    result << fullPath;
                    if (doReturn()) {
                        return;
                    }
                }

                if (!!(flags & PathFinder::RECURSIVE)) {
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

    if (!!(flags & PathFinder::USE_SYSTEM_PATHS)) {
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
    rename(source.toStdString().c_str(), destination.toStdString().c_str());
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
