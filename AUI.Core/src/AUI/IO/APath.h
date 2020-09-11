#pragma once

#include <iterator>
#include <AUI/Common/AString.h>
#include <dirent.h>
#include <AUI/Util/EnumUtil.h>


ENUM_FLAG(ListFlags) {
    LF_NONE = 0,

    /**
     * \brief Некоторые файловые системы включают "." и ".." в список файлов. В AUI по умолчанию пропускаются эти
     *        элементы. Этот флаг отменяет исключение ".", ".."
     */
    LF_DONT_IGNORE_DOTS = 1,

    /**
     * \brief Включать папки в список.
     */
    LF_DIRS = 2,

    /**
     * \brief Включать обычные файлы в список.
     */
    LF_REGULAR_FILES = 4,

    LF_DEFAULT_FLAGS = LF_DIRS | LF_REGULAR_FILES
};

/**
 * \brief Надстройка над AString с функциями для работы с путём.
 * \note Иногда под словом "файл" ("file") имеется в виду и <i>обычный файл</i> (txt, png, jpeg и т.д.), и <i>папка</i>
 *       (директория, каталог, которая содержать другие файлы и папки), т.е. единица файловой системы, из-за чего часто
 *       возникает путанница в терминологии. Здесь и далее:
 *       <ul>
 *          <li><b>файл</b> (file) - единица файловой системы.</li>
 *          <li><b>обычный файл</b> (regular file) - файл, который можно прочитать или в который можно записать. Можно представить
 *              как последовательность байт или поток байтов.</li>
 *          <li><b>папка</b> (directory) - файл, который включает себя другие файлы (обычные файлы и папки)</li>
 *       </ul>
 * \note В большинстве файловых систем по одному пути может существовать и обычный файл, и папка с одинаковымиъ
 *       названиями.
 */
class APath: public AString {
private:
    APath ensureSlashEnding() const;
    APath ensureNonSlashEnding() const;

public:
    APath() = default;
    APath(AString&& other);
    APath(const AString& other): AString(other) {}
    APath(const char* str): AString(str) {}
    APath(const wchar_t * str): AString(str) {}

    /**
     * \brief Составить список имён файлов
     * \note Некоторые файловые системы включают "." и ".." в список файлов. В AUI по умолчанию пропускаются эти
     *       элементы. Используйте L_DONT_IGNORE_DOTS, чтобы отменить пропуск папок "." и ".."
     * \return Список имён файлов в папке (имён, не путей)
     */
    ADeque<AString> listDir(ListFlags f = LF_DEFAULT_FLAGS) const;

    /**
     * \example <pre>/home/user -> /home</pre>
     * \return путь родительской папки
     */
    [[nodiscard]] APath up() const;

    /**
     * \brief Путь дочернего элемента. Актуально только для папок.
     * \example при fileName = work: <pre>/home/user -> /home/user/work</pre>
     * \param название дочернего файла
     * \return путь до дочернего файла
     */
    [[nodiscard]] APath file(const AString& fileName) const;

    /**
     * \brief Название файла.
     * \example <pre>/home/user/file.cpp -> file.cpp
     * \return название файла
     */
    [[nodiscard]] AString filename() const;

    /**
     * \return true, если обычный файл или папка существует
     * \note файл может существовать как обычный файл, как папка, или и то, и другое. Эта функция вернёт false только
     *       в том случае, если ни папки, ни файла по этому пути не существует.
     */
    bool exists() const;

    /**
     * \return true, если обычный файл существует
     * \note файл может существовать как обычный файл, как папка, или и то, и другое. Эта функция вернёт true только
     *       в том случае, если по этому пути существует именно обычный файл.
     */
    bool isRegularFileExists() const;


    /**
     * \return true, если папка существует
     * \note файл может существовать как обычный файл, как папка, или и то, и другое. Эта функция вернёт true только
     *       в том случае, если по этому пути существует именно папка.
     */
    bool isDirectoryExists() const;

    /**
     * \brief Удалить файлы рекурсивно. Актуально для папок.
     * \return this
     */
    const APath& removeFile() const;

    /**
     * \brief Удалить файлы рекурсивно. Актуально для папок.
     * \return this
     */
    const APath& removeFileRecursive() const;
};

