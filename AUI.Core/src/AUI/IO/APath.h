#pragma once

#include <iterator>
#include <AUI/Common/AString.h>
#include <AUI/Util/EnumUtil.h>


ENUM_FLAG(ListFlags) {
    LF_NONE = 0,

    /**
     * \brief Some file systems include ". " and " .. " to the list of files. In AUI, these elements are skipped by
     *        default. This flag overrides this behaviour.
     */
    LF_DONT_IGNORE_DOTS = 1,

    /**
     * \brief Include folders to the list of files.
     */
    LF_DIRS = 2,

    /**
     * \brief Include regular files to the list of files.
     */
    LF_REGULAR_FILES = 4,

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
    LF_RECURSIVE = 8,

    LF_DEFAULT_FLAGS = LF_DIRS | LF_REGULAR_FILES
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
class API_AUI_CORE APath: public AString {
private:
    APath ensureSlashEnding() const;
    APath ensureNonSlashEnding() const;

    void removeBackSlashes();

public:
    APath() = default;
    APath(AString&& other): AString(other) {
        removeBackSlashes();
    }
    APath(const AString& other): AString(other) {
        removeBackSlashes();
    }
    APath(const char* str): AString(str) {
        removeBackSlashes();
    }
    APath(const wchar_t * str): AString(str) {
        removeBackSlashes();
    }

    /**
     * \brief Get the absolute (full) path to the file.
     * \return the absolute (full) path to the file
     */
    APath absolute() const;

    /**
     * \brief Get list of (by default) direct children of this folder.
     * \note Use ListFlags enum flags to customize behaviour of this function.
     * \return list of children of this folder.
     */
    ADeque<APath> listDir(ListFlags f = LF_DEFAULT_FLAGS) const;

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

    size_t fileSize() const;

    enum DefaultPath {
        /**
         * \brief Folder for application data.
         * Windows: C:/Users/%user%/.appdata/Roaming/
         * Linux: %homedir%/.local/share/
         */
        APPDATA,

        /**
         * \brief Folder for temporary data.
         * Windows: User's temp folder (%temp%)
         * Linux: /tmp
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
     * \return working dir of application
     */
    static APath workingDir();
};

