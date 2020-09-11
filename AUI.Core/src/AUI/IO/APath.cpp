//
// Created by alex2772 on 9/10/20.
//

#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include "APath.h"
#include "IOException.h"
#include "FileNotFoundException.h"

APath::APath(AString&& other) : AString(other) {}

APath APath::up() const {
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
    struct stat s;
    stat(toStdString().c_str(), &s);

    return S_ISDIR(s.st_mode) || S_ISREG(s.st_mode);
}
bool APath::isRegularFileExists() const {
    struct stat s;
    stat(toStdString().c_str(), &s);

    return S_ISREG(s.st_mode);
}

bool APath::isDirectoryExists() const {
    struct stat s;
    stat(toStdString().c_str(), &s);

    return S_ISDIR(s.st_mode);
}

const APath& APath::removeFile() const {
    if (::remove(toStdString().c_str()) != 0) {
        throw IOException(strerror(errno));
    }
    return *this;
}

const APath& APath::removeFileRecursive() const {
    if (isDirectoryExists()) {

    }
    removeFile();
    return *this;
}

ADeque<AString> APath::listDir(ListFlags f) const {
    ADeque<AString> list;

    DIR* dir = opendir(toStdString().c_str());

    for (dirent* i; (i = readdir(dir));) {
        if (!(f & LF_DONT_IGNORE_DOTS)) {
            if ("."_as == i->d_name || ".."_as == i->d_name) {
                continue;
            }
        }
        if (!(f & LF_DIRS) && i->d_type & DT_DIR) {
            continue;
        }
        if (!(f & LF_REGULAR_FILES) && i->d_type & DT_REG) {
            continue;
        }
        list << i->d_name;
    }
    closedir(dir);
    return list;
}