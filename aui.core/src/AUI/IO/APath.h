/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <iterator>
#include "AUI/Reflect/AEnumerate.h"
#include <AUI/Common/AString.h>
#include <AUI/Common/ADeque.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/serializable.h>

/**
 * @brief Flag enum for APath::find
 * @ingroup io
 */
AUI_ENUM_FLAG(APathFinder) {
    NONE,

    /**
     * In addition to specified paths, use the system paths (PATH environment variable)
     */
    USE_SYSTEM_PATHS = 1 << 0,

    /**
     * Do scan recursively (slow)
     */
    RECURSIVE = 1 << 1,

    /**
     * Return only one file
     */
    SINGLE = 1 << 2
};

/**
 * @brief Flag enum for APath::listDir
 * @ingroup io
 */
AUI_ENUM_FLAG(AFileListFlags) {
    NONE = 0,

    /**
     * @brief Some file systems include ". " and " .. " to the list of files. In AUI, these elements are skipped by
     *        default. This flag overrides this behaviour.
     */
    DONT_IGNORE_DOTS = 1 << 0,

    /**
     * @brief Include folders to the list of files.
     */
    DIRS = 1 << 1,

    /**
     * @brief Include regular files to the list of files.
     */
    REGULAR_FILES = 1 << 2,

    /**
     * @brief Walk thru the folder recursively (i.e. include the contents of child folders). The paths of child files
     * are set relative to the folder where the <code>listDir()</code> is called.
    * @details
     *
     * <ul>
     *     <li>/home</li>
     *     <li>/home/user</li>
     *     <li>/home/user/file1.txt</li>
     *     <li>/home/user/file2.txt</li>
     *     <li>/home/other</li>
     *     <li>/home/other/code1.cpp</li>
     *     <li>/home/other/code2.cpp</li>
     * </ul>
     */
    RECURSIVE = 1 << 3,

    DEFAULT_FLAGS = AFileListFlags::DIRS | AFileListFlags::REGULAR_FILES
};

/**
 * @brief Flag enum for APath::isEffectivelyAccessible
 * @ingroup io
 */
AUI_ENUM_FLAG(AFileAccess) {
    /**
     * @brief File is Readable flag.
     */
    R = 0b100,

    /**
     * @brief File is Writeable flag.
     */
    W = 0b010,

    /**
     * @brief File is eXecutable flag.
     */
    X = 0b001,
};

/**
 * @brief An add-on to AString with functions for working with the path.
 * @ingroup io
 * @note In most file systems, both a regular file and a folder with the same name can exist on the same path.
 * @details
 * Example usage:
 * @code{cpp}
 * APath someDir = "someDir";
 * APath filePath = someDir / "myfile.txt"; // "/" replaced with a system file separator
 * @endcode
 * @note Sometimes the word "file" refers to both a <i>regular file</i> (txt, png, jpeg, etc.) and a <i>folder</i>
 *       (directory, a file that contains other regular files and folders), i.e. a unit of the file system, which is
 *       often a confusion in terminology. Here and further:
 *       <ul>
 *          <li><b>file</b> - a unit of the file system.</li>
 *          <li><b>regular file</b> - a file that can be read or written to. You can think of as a sequence of bytes
 *                                    or a stream of bytes.</li>
 *          <li><b>folder</b> (directory) - a file that may have child files (both regular files and folders)</li>
 *       </ul>
 */
class API_AUI_CORE APath final: public AString {
private:
    APath ensureSlashEnding() const;
    APath ensureNonSlashEnding() const;

    void removeBackSlashes();


#if AUI_PLATFORM_WIN
    struct _stat64 stat() const;
#else
    struct stat stat() const;
#endif

public:
    APath() = default;
    APath(AString&& other) noexcept: AString(std::move(other)) {
        removeBackSlashes();
    }
    APath(const AString& other) noexcept: AString(other) {
        removeBackSlashes();
    }
    APath(const char* utf8) noexcept: AString(utf8) {
        removeBackSlashes();
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    APath(const char* utf8, std::size_t length) noexcept: AString(utf8, utf8 + length) {
        removeBackSlashes();
    }
    APath(const char16_t * str) noexcept: AString(str) {
        removeBackSlashes();
    }

    APath(const char16_t * str, std::size_t length) noexcept: AString(str, str + length) {
        removeBackSlashes();
    }

    /**
     * @brief Generates a unique, process-agnostic temporary directory in the system's temp directory.
     * @details
     * Creates a safe and islocated workspace for each application instance. By generating a new directory for each
     * process, it prevents, potential conflicts between concurrent processes.
     *
     * When the application closes, a directory cleanup attempt will be performed.
     *
     * @sa APath::nextRandomTemporary
     *
     * @return Path to a process-agnostic empty pre-created directory in system temp directory.
     */
    [[nodiscard]]
    static const APath& processTemporaryDir();

    /**
     * @brief Creates a path to non-existent random file in system temp directory.
     * @details
     * The file is guaranteed to be non-existent, however, its parent directory does. The such path can be used for
     * general purposes. The application might create any kind of file on this location (including dirs) or don't create
     * any file either.
     * @sa APathOwner
     * @sa APath:processTemporaryDir:
     */
    [[nodiscard]]
    static APath nextRandomTemporary();

    /**
     * Creates a file.
     * @return this.
     */
    const APath& touch() const;

    /**
     * @brief Transforms this path to string with platform's native slashes.
     * @details
     * @specificto{windows}
     * Returns path with backward slashes `\\`.
     *
     * On any other platform, returns path with forward slashes `/`.
     */
    AString systemSlashDirection() const;

    /**
     * @brief Get the absolute (full) path to the file.
     * @return the absolute (full) path to the file
     */
    APath absolute() const;

    /**
     * @brief Get list of (by default) direct children of this folder. This function outputs paths including the path
     *        listDir was called on.
     * @note Use AFileListFlags enum flags to customize behaviour of this function.
     * @sa relativelyTo
     * @return list of children of this folder.
     */
    ADeque<APath> listDir(AFileListFlags f = AFileListFlags::DEFAULT_FLAGS) const;

    /**
     * @details
     * `/home/user -> /home`
     * @return path to parent folder
     */
    [[nodiscard]] APath parent() const;

    /**
     * @brief Path of the child element. Relevant only for folders.
     * @details
     * with `fileName = work`: `/home/user -> /home/user/work`
     * @note
     * It's convient to use `/` syntax instead:
     * @code{cpp}
     * APath("/home/user") / "work"
     * @endcode
     *
     * @param fileName name of child file
     * @return path to child file relatively to this folder
     */
    [[nodiscard]] APath file(const AString& fileName) const;

    /**
     * @brief File name.
     * @details
     * `/home/user/file.cpp -> file.cpp`
     * @return file name
     */
    [[nodiscard]] APath filename() const;

    /**
     * @brief File name without extension.
     * @details
     * `/home/user/file.cpp -> file
     * @return file name without extension
     */
    [[nodiscard]] APath filenameWithoutExtension() const;

    /**
     * @brief File extension.
     * @details
     * `/home/user/file.cpp -> cpp
     * @return file extension
     */
    [[nodiscard]] AString extension() const;

    /**
     * @brief Remove the uppermost folder from this path
     * @details
     * v1.0.0/client/azaza.zip -> client/azaza.zip
     * @return The same path except uppermost folder
     */
    [[nodiscard]] APath withoutUppermostFolder() const;

    /**
     * @return true if whether regular file or a folder exists on this path
     * @note A file can exist as a regular file or(and) as a folder. This function will return false only if neither
     *       the folder nor the file does not exists on this path.
     *
     *       Checkout the <code>isRegularFileExists</code> or <code>isDirectoryExists</code> function to check which
     *       type of the file exists on this path.
     */
    bool exists() const;


    /**
     * @return true if regular file exists on this path
     * @note A file can exist as a regular file or(and) as a folder. This function will return false only if regular
     *       file does not exists on this path.
     */
    bool isRegularFileExists() const;

    /**
     * @return true if folder exists on this path
     * @note A file can exist as a regular file or(and) as a folder. This function will return false only if folder does
     *       not exists on this path.
     */
    bool isDirectoryExists() const;

    /**
     * @brief Delete file. Relevant for empty folders and regular files.
     * @return this
     * @details
     * Unlike remove*Recursive functions, this function has no checks before proceeding, thus, it might throw
     * AIOException (including if the target does not exist).
     *
     * @sa APath::removeFileRecursive()
     * @sa APath::removeDirContentsRecursive()
     */
    const APath& removeFile() const;

    /**
     * @brief Delete files recursively, including itself.
     * @return this
     * @details
     * If this APath points to a regular file, it deletes the file directly. If this APath points to a directory, it
     * first removes all its contents (recursively) before potentially deleting the directory itself.
     *
     * If the target does not exist, this function has no effect.
     *
     * @sa APath::removeDirContentsRecursive()
     */
    const APath& removeFileRecursive() const;

    /**
     * @brief Delete directory contents (recursively).
     * @return this
     * @details
     * If this APath points to a regular file, the function has no effect. If this APath points to a directory, it
     * removes all contained files (recursively) within that directory but does not remove the directory itself.
     *
     * If the target does not exist, this function has no effect.
     *
     * @sa APath::removeFileRecursive()
     */
    const APath& removeDirContentsRecursive() const;

    /**
     * @brief Create folder.
     * @return this
     */
    const APath& makeDir() const;

    /**
     * @brief Create all nonexistent folders on the path.
     * @return this
     */
    const APath& makeDirs() const;

    /**
     * @brief Returns same path but without <code>dir</code>
     * @param dir some parent, grandparent, grandgrandparent... dir
     * @details
     * APath("C:/work/mon/test.txt").relativelyTo("C:/work") -> mon/test.txt
     * @return same path but without <code>dir</code>
     */
    AString relativelyTo(const APath& dir) const;

    /**
     * @brief Returns same path but with extension changed.
     */
    APath extensionChanged(const AString& newExtension) const;

    /**
     * @brief Checks whether path absolute or not.
     * @return true if path is absolute
     */
    bool isAbsolute() const;
    /**
     * @brief Checks whether path absolute or not.
     * @return true if path is relative
     */
    bool isRelative() const {
        return !isAbsolute();
    }

    time_t fileModifyTime() const;
    size_t fileSize() const;

    /**
     * @brief Changes mode (permissions) on file
     * @param newMode new mode.
     * @details
     * It's convenient to use octet literal on `newMode`:
     * @code{cpp}
     * APath p("file.txt");
     * p.chmod(0755); // -rwxr-xr-x
     * @endcode
     */
    const APath& chmod(int newMode) const;

    enum DefaultPath {
        /**
         * @brief Folder for application data.
         * @details
         * @specificto{windows}
         * Maps to `C:/Users/ %user% /.appdata/Roaming/`.
         *
         * @specificto{linux}
         * Maps to `$HOME/.local/share/`.
         *
         * @specificto{android}
         * Maps to `\<internal_storage_path\>/__aui_appdata`.
         *
         * @specificto{ios}
         * Maps to `\<internal_storage_path\>/__aui_appdata`.
         */
        APPDATA,

        /**
         * @brief Folder for temporary data.
         * @details
         * @specificto{windows}
         * Maps to user's temp folder `%temp%`.
         *
         * @specificto{linux}
         * Maps to system temp directory `/tmp`.
         *
         * @specificto{macos}
         * Maps to system temp directory `/tmp`.
         *
         * @specificto{android}
         * Maps to AUI-managed temporary directory: `\<internal_storage_path\>/__aui_tmp`.
         *
         * @specificto{ios}
         * Maps to AUI-managed temporary directory: `\<internal_storage_path\>/__aui_tmp`.
         */
        TEMP,

        /**
         * @brief User home directory.
         * @details
         * @specificto{windows}
         * Maps to user's home folder `C:\\Users\\ %user%`.
         *
         * @specificto{linux}
         * Maps to user's home folder `/home/$USER`.
         */
        HOME,
    };

    /**
     * @brief Get system's default folder.
     * @note See the <code>APath::DefaultPath</code> definition.
     * @return absolute path to default folder.
     */
    static APath getDefaultPath(DefaultPath path);

    /**
     * @brief Copy regular file.
     * @param source source file
     * @param destination destination file
     */
    static void copy(const APath& source, const APath& destination);

    /**
     * @brief Move regular file.
     * @param source source file
     * @param destination destination file
     */
    static void move(const APath& source, const APath& destination);

    /**
     * @return working dir of application
     */
    static APath workingDir();


    /**
     * Searches for file in specified dirs.
     * @param filename Name of the file searching for
     * @param locations paths to directories to search for the file in
     * @param flags lookup flags (see APathFinder)
     * @return full path to the found file; if file not found, an empty string is returned.
     */
    static AVector<APath> find(const AString& filename, const AVector<APath>& locations, APathFinder flags = APathFinder::NONE);

    /**
     * @brief Path of the child element. Relevant only for folders.
     * @param filename child to produce path to
     * @code{cpp}
     * AString filename = "file.txt";
     * APath path = "path" / "to" / "your" / filename;
     * @endcode
     * Which would supplyValue into "path/to/your/file.txt"
     * @return path to child file relatively to this folder
     */
    [[nodiscard]]
    APath operator/(const AString& filename) const {
        return file(filename);
    }

    /**
     * @brief Return true if the current process has specified access flags to path.
     * @details
     * Checks permissions and existence of the file identified by this APath using the real user and group
     * identifiers of the process, like if the file were opened by open().
     *
     * @note
     * Using this function to check a process's permissions on a file before performing some operation based on that
     * information leads to race conditions: the file permissions may change between the two steps. Generally, it is
     * safer just to attempt the desired operation and handle any permission error that occurs.
     */
    [[nodiscard]]
    bool isEffectivelyAccessible(AFileAccess flags) const noexcept;
};

/**
 * @brief RAII-style file owner for storing temporary data on disk.
 * @ingroup io
 * @details
 * This class represents a type that transparently converts to underlying APath. When APathOwner is destructed, the
 * pointed file is cleaned up, too, regardless of it's type.
 *
 * APathOwner is designed to simplify management of (temporary) files on disk, ensuring cleanup of the pointed file
 * in RAII (Resource Acquisition Is Initialization) style.
 *
 * @snippet aui.updater/src/AUI/Updater/AUpdater.cpp APathOwner example
 * @sa APath::nextRandomTemporary()
 */
struct API_AUI_CORE APathOwner: public aui::noncopyable {
public:
    explicit APathOwner(APath mPath) noexcept: mPath(std::move(mPath)) {}

    ~APathOwner() {
        try {
            mPath.removeFileRecursive();
        } catch(...) {}
    }

    [[nodiscard]]
    operator const APath&() const noexcept {
        return mPath;
    }

    [[nodiscard]]
    const APath& value() const noexcept {
        return mPath;
    }

private:
    APath mPath;
};

inline APath operator""_path(const char* str, std::size_t length) {
    return APath(str, length);
}

template<>
struct ASerializable<APath>: ASerializable<AString> {};
