//
// Created by alex2772 on 9/10/20.
//

#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include "APath.h"
#include "IOException.h"
#include "FileNotFoundException.h"

#ifdef WIN32
#include <Windows.h>
#include <direct.h>

#define ERROR_DESCRIPTION
#else
#include <dirent.h>
#define ERROR_DESCRIPTION  + ": " + strerror(errno)
#endif

APath::APath(AString&& other) : AString(other) {}

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
    struct stat s = {0};
    stat(toStdString().c_str(), &s);

    return s.st_mode & (S_IFDIR | S_IFREG);
}
bool APath::isRegularFileExists() const {
    struct stat s = {0};
    stat(toStdString().c_str(), &s);

    return s.st_mode & S_IFREG;
}

bool APath::isDirectoryExists() const {
    struct stat s = {0};
    stat(toStdString().c_str(), &s);

    return s.st_mode & S_IFDIR;
}

const APath& APath::removeFile() const {
    if (::remove(toStdString().c_str()) != 0) {
        throw IOException("could not remove file " + *this ERROR_DESCRIPTION);
    }
    return *this;
}

const APath& APath::removeFileRecursive() const {
    if (isDirectoryExists()) {
        for (auto& l : listDir()) {
            file(l).removeFileRecursive();
        }
    }
    if (exists())
        removeFile();
    return *this;
}

ADeque<AString> APath::listDir(ListFlags f) const {
    ADeque<AString> list;

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
#else
    for (dirent* i; (i = readdir(dir));) {
        auto& filename = i->d_name;
#endif

        if (!(f & LF_DONT_IGNORE_DOTS)) {
            if ("."_as == filename || ".."_as == filename) {
                continue;
            }
        }
        if ((f & LF_DIRS && i->d_type & DT_DIR) || (f & LF_REGULAR_FILES && i->d_type & DT_REG)) {
            list << filename;
        }
        if (f & LF_RECURSIVE && i->d_type & DT_DIR) {
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
    closedir(dir);
    return list;
}

APath APath::absolute() {
    char buf[0x1000];
    if (realpath(toStdString().c_str(), buf) == nullptr) {
        throw IOException("could not find absolute file" + *this ERROR_DESCRIPTION);
    }

    return buf;
}

const APath& APath::makeDir() const {
    if (::mkdir(toStdString().c_str(), 0755) != 0) {
        throw IOException("could not create directory: "_as ERROR_DESCRIPTION);
    }
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
