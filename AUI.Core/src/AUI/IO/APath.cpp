//
// Created by alex2772 on 9/10/20.
//

#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include "APath.h"
#include "IOException.h"
#include "FileNotFoundException.h"
#include "InsufficientPermissionsException.h"

#ifdef WIN32
#include <windows.h>
#include <direct.h>

#define ERROR_DESCRIPTION
#else
#include <dirent.h>
#define ERROR_DESCRIPTION  + ": " + strerror(errno)
#endif

APath APath::parent() const {
    auto c = ensureNonSlashEnding().rfind(L'/');
    if (c != NPOS) {
        return mid(0, c);
    }
    return {};
}

AString APath::filename() const {
     auto i = rfind(L'/');
     if (i == NPOS) {
         return *this;
     }
    return mid(i + 1);
}

AString APath::filenameWithoutExtension() const {
    auto name = filename();
    auto it = name.rfind('.');
    if (it == NPOS) {
        return name;
    }
    return name.mid(0, it);
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
        return mid(0, length() - 1);
    }
    return *this;
}

bool APath::exists() const {
#ifdef _WIN32
    struct _stat64 s = {0};
    _wstat64(c_str(), &s);
#else
    struct stat s = {0};
    stat(toStdString().c_str(), &s);
#endif

    return s.st_mode & (S_IFDIR | S_IFREG);
}
bool APath::isRegularFileExists() const {
#ifdef _WIN32
    struct _stat64 s = {0};
    _wstat64(c_str(), &s);
#else
    struct stat s = {0};
    stat(toStdString().c_str(), &s);
#endif

    return s.st_mode & S_IFREG;
}

bool APath::isDirectoryExists() const {
#ifdef _WIN32
    struct _stat64 s = {0};
    _wstat64(c_str(), &s);
#else
    struct stat s = {0};
    stat(toStdString().c_str(), &s);
#endif

    return s.st_mode & S_IFDIR;
}

const APath& APath::removeFile() const {
#ifdef _WIN32
    if (::_wremove(c_str()) != 0) {
#else
    if (::remove(toStdString().c_str()) != 0) {
#endif
        throw IOException("could not remove file " + *this ERROR_DESCRIPTION);
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
    HANDLE dir = FindFirstFile(file("*").c_str(), &fd);

    if (dir == INVALID_HANDLE_VALUE)
#else
    DIR* dir = opendir(toStdString().c_str());
    if (!dir)
#endif
        throw FileNotFoundException("could not list " + *this ERROR_DESCRIPTION);

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

        if (!(f & LF_DONT_IGNORE_DOTS)) {
            if ("."_as == filename || ".."_as == filename) {
                continue;
            }
        }
        if ((f & LF_DIRS && isDirectory) || (f & LF_REGULAR_FILES && isFile)) {
            list << file(filename);
        }
        if (f & LF_RECURSIVE && isDirectory) {
            auto childDir = file(filename);
            for (auto& file : childDir.listDir(f)) {
                if (file.startsWith(childDir)) {
                    // абсолютный путь
                    auto p = file.mid(childDir.length());
                    if (p.startsWith("/")) {
                        p = p.mid(1);
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
#ifdef WIN32
    APath r;
    r.resize(0x1000);
    if (_wfullpath(r.data(), c_str(), r.length()) == nullptr) {
        throw IOException("could not find absolute file" + *this ERROR_DESCRIPTION);
    }

    r.resizeToNullTerminator();

    return r;
#else
    char buf[0x1000];
    if (realpath(toStdString().c_str(), buf) == nullptr) {
        throw IOException("could not find absolute file" + *this ERROR_DESCRIPTION);
    }

    return buf;
#endif
}

const APath& APath::makeDir() const {
#ifdef WIN32
    //                           VV - КОЗЛЫ, МЛЯТЬ!
    if (::_wmkdir(c_str()) == 0) {
        auto s = "could not create directory: "_as + absolute() ERROR_DESCRIPTION;
        auto et = GetLastError();
        switch (et) {
            case ERROR_ACCESS_DENIED:
                throw InsufficientPermissionsException(s);
            case ERROR_ALREADY_EXISTS:
                break;
            default:
                throw IOException(s);
        }
    }
#else
    if (::mkdir(toStdString().c_str(), 0755) != 0) {
        throw IOException("could not create directory: "_as + absolute() ERROR_DESCRIPTION);
    }
#endif
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

#ifdef _WIN32
#include <shlobj.h>

APath APath::getDefaultPath(APath::DefaultPath path) {
    APath result;
    result.resize(0x800);
    switch (path) {
        case APPDATA:
            SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, result.data());
            break;
        case TEMP:
            GetTempPath(result.length(), result.data());
            break;
        default:
            assert(0);
    }
    result.resizeToNullTerminator();
    result.removeBackSlashes();
    return result;
}

size_t APath::fileSize() const {
#ifdef _WIN32
    struct _stat64 s = {0};
    _wstat64(c_str(), &s);
#else
    struct stat s = {0};
    stat(toStdString().c_str(), &s);
#endif
    return s.st_size;
}

APath APath::withoutUppermostFolder() const {
    auto r = find('/');
    if (r == NPOS)
        return *this;
    return mid(r + 1);
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

APath APath::getDefaultPath(APath::DefaultPath path) {
    switch (path) {
        case APPDATA:
            return APath(getpwuid(getuid())->pw_dir).file(".local/share");

        case TEMP:
            return "/tmp";
        default:
            assert(0);
    }
    return {};
}
#endif