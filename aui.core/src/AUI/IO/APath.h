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

#pragma once

#include <iterator>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Util/EnumUtil.h>


/**
 * Flag enum for APath::find
 */
ENUM_FLAG(PathFinder) {
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

ENUM_FLAG(ListFlags) {
    NONE = 0,

    /**
     * \brief Some file systems include ". " and " .. " to the list of files. In AUI, these elements are skipped by
     *        default. This flag overrides this behaviour.
     */
    DONT_IGNORE_DOTS = 1 << 0,

    /**
     * \brief Include folders to the list of files.
     */
    DIRS = 1 << 1,

    /**
     * \brief Include regular files to the list of files.
     */
    REGULAR_FILES = 1 << 2,

    /**
     * \brief Walk thru the folder recursively (i.e. include the contents of child folders). The paths of child files
     * are set relative to the folder where the <code>listDir()</code> is called.
     * \example
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

    DEFAULT_FLAGS = ListFlags::DIRS | ListFlags::REGULAR_FILES
};

/**
 * \brief An add-on to AString with functions for working with the path.
 * \note Sometimes the word "file" refers to both a <i>regular file</i> (txt, png, jpeg, etc.) and a <i>folder</i>
 *       (directory, a file that contains other regular files and folders), i.e. a unit of the file system, which is
 *       often a confusion in terminology. Here and further:
 *       <ul>
 *          <li><b>file</b> - a unit of the file system.</li>
 *          <li><b>regular file</b> - a file that can be read or written to. You can think of as a sequence of bytes
 *                                    or a stream of bytes.</li>
 *          <li><b>folder</b> (directory) - a file that may have child files (both regular files and folders)</li>
 *       </ul>
 * \note In most file systems, both a regular file and a folder with the same name can exist on the same path.
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
    APath(AString&& other) noexcept: AString(std::forward<AString>(other)) {
        removeBackSlashes();
    }
    APath(const AString& other) noexcept: AString(other) {
        removeBackSlashes();
    }
    APath(const char* str) noexcept: AString(str) {
        removeBackSlashes();
    }

    APath(AStringView stringView) noexcept: AString(stringView) {
        removeBackSlashes();
    }

    /**
     * Creates a file.
     * @return this.
     */
    const APath& touch() const;

    /**
     * @return On Linux/macOS, returns self copy. On Windows, returns self copy with backslashes (\) instead of slashes (/).
     */
    AString systemSlashDirection() const;

    /**
     * \brief Get the absolute (full) path to the file.
     * \return the absolute (full) path to the file
     */
    APath absolute() const;

    /**
     * \brief Get list of (by default) direct children of this folder. This function outputs paths including the path
     *        listDir was called on.
     * \note Use ListFlags enum flags to customize behaviour of this function.
     * \return list of children of this folder.
     */
    ADeque<APath> listDir(ListFlags f = ListFlags::DEFAULT_FLAGS) const;

    /**
     * \example <pre>/home/user -> /home</pre>
     * \return path to parent folder
     */
    [[nodiscard]] APath parent() const;

    /**
     * \brief Path of the child element. Relevant only for folders.
     * \example with fileName = work: <pre>/home/user -> /home/user/work</pre>
     * \param name of child file
     * \return path to child file relatively to this folder
     */
    [[nodiscard]] APath file(const AString& fileName) const;

    /**
     * \brief Path of the child element. Relevant only for folders.
     * \example with fileName = work: <pre>/home/user -> /home/user/work</pre>
     * \param name of child file
     * \return path to child file relatively to this folder
     */
    [[nodiscard]] APath operator[](const AString& fileName) const {
        return file(fileName);
    }

    /**
     * \brief File name.
     * \example <pre>/home/user/file.cpp -> file.cpp
     * \return file name
     */
    [[nodiscard]] AString filename() const;

    /**
     * \brief File name without extension.
     * \example <pre>/home/user/file.cpp -> file
     * \return file name without extension
     */
    [[nodiscard]] AString filenameWithoutExtension() const;

    /**
     * \brief Remove the uppermost folder from this path
     * \example v1.0.0/client/azaza.zip -> client/azaza.zip
     * \return The same path except uppermost folder
     */
    [[nodiscard]] APath withoutUppermostFolder() const;

    /**
     * \return true if whether regular file or a folder exists on this path
     * \note A file can exist as a regular file or(and) as a folder. This function will return false only if neither
     *       the folder nor the file does not exists on this path.
     *
     *       Checkout the <code>isRegularFileExists</code> or <code>isDirectoryExists</code> function to check which
     *       type of the file exists on this path.
     */
    bool exists() const;


    /**
     * \return true if regular file exists on this path
     * \note A file can exist as a regular file or(and) as a folder. This function will return false only if regular
     *       file does not exists on this path.
     */
    bool isRegularFileExists() const;

    /**
     * \return true if folder exists on this path
     * \note A file can exist as a regular file or(and) as a folder. This function will return false only if folder does
     *       not exists on this path.
     */
    bool isDirectoryExists() const;

    /**
     * \brief Delete file. Relevant for empty folders and regular files.
     * \return this
     */
    const APath& removeFile() const;

    /**
     * \brief Delete files recursively. Relevant for folders.
     * \return this
     */
    const APath& removeFileRecursive() const;

    /**
     * \brief Create folder.
     * \return this
     */
    const APath& makeDir() const;

    /**
     * \brief Create all nonexistent folders on the path.
     * \return this
     */
    const APath& makeDirs() const noexcept;

    /**
     * \brief Returns same path but without <code>folder</code>
     * \param dir some parent, grandparent, grandgrandparent... dir
     * \example APath("C:/work/mon/test.txt").relativelyTo("C:/work") -> mon/test.txt
     * \return same path but without <code>dir</code>
     */
    AString relativelyTo(const APath& dir) const;

    /**
     * \brief Checks whether path absolute or not.
     * \return true if path is absolute
     */
    bool isAbsolute() const;
    /**
     * \brief Checks whether path absolute or not.
     * \return true if path is relative
     */
    bool isRelative() const {
        return !isAbsolute();
    }

    time_t fileModifyTime() const;
    size_t fileSize() const;

    enum DefaultPath {
        /**
         * \brief Folder for application data.
         * Windows: C:/Users/%user%/.appdata/Roaming/
         * Linux: %homedir%/.local/share/
         * Android: <internal_storage_path>/__aui_appdata
         */
        APPDATA,

        /**
         * \brief Folder for temporary data.
         * Windows: User's temp folder (%temp%)
         * Linux: /tmp
         * Android: <internal_storage_path>/__aui_tmp
         */
        TEMP,
    };

    /**
     * \brief Get system's default folder.
     * \note See the <code>APath::DefaultPath</code> definition.
     * \return absolute path to default folder.
     */
    static APath getDefaultPath(DefaultPath path);

    /**
     * \brief Copy regular file.
     * \param source source file
     * \param destination destination file
     */
    static void copy(const APath& source, const APath& destination);

    /**
     * \brief Move regular file.
     * \param source source file
     * \param destination destination file
     */
    static void move(const APath& source, const APath& destination);

    /**
     * \return working dir of application
     */
    static APath workingDir();


    /**
     * Searches for file in specified dirs.
     * @param filename Name of the file searching for
     * @param locations paths to directories to search for the file in
     * @return full path to the found file; if file not found, an empty string is returned.
     */
    static AVector<APath> find(const AString& filename, const AVector<APath>& locations, PathFinder flags = PathFinder::NONE);
};

