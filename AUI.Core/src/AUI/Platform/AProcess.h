//
// Created by alex2 on 31.10.2020.
//

#pragma once


#include <AUI/IO/APath.h>
#include <AUI/Common/AException.h>

#if defined(_WIN32)
#include <windows.h>
#endif

class AProcessException: public AException {
public:
    AProcessException(const AString& message): AException(message) {}
};

class API_AUI_CORE AProcess {
private:
    AString mApplicationFile;
    AString mArgs;
    APath mWorkingDirectory;

#if defined(_WIN32)
    PROCESS_INFORMATION mProcessInformation;
#endif

public:
    AProcess(const AString& applicationFile) : mApplicationFile(applicationFile) {}
    const AString& getApplicationFile() const {
        return mApplicationFile;
    }

    const AString& getArgs() const {
        return mArgs;
    }

    void setArgs(const AString& args) {
        mArgs = args;
    }

    const APath& getWorkingDirectory() const {
        return mWorkingDirectory;
    }

    void setWorkingDirectory(const APath& workingDirectory) {
        mWorkingDirectory = workingDirectory;
    }

    /**
     * \brief Запустить процесс.
     */
    void run();

    /**
     * \brief Подождать завершения процесса.
     */
    void wait();

    /**
     * \brief Подождать завершения процесса и получить exit-code процесса.
     * \return exit-code процесса
     */
    int getExitCode();

    /**
     * \brief Запустить указанный исполяемый файл с указанными аргументами.
     * \param applicationFile исполняемый файл
     * \param args аргументы
     * \param workingDirectory рабочая директория создаваемого процесса
     * \param waitForExit ожидать завершения процесса. Если false, то функция вернёт 0, так как exit-code процесса
     *        неизвестен в момент его выполнения
     * \return exit-code процесса
     */
    static int execute(const AString& applicationFile,
                       const AString& args = {},
                       const APath& workingDirectory = {},
                       bool waitForExit = true);


    /**
     * \brief Запустить указанный исполяемый файл от имени адмнистратор с указанными аргументами.
     * \param applicationFile исполняемый файл
     * \param args аргументы
     * \param workingDirectory рабочая директория создаваемого процесса
     * \note Эта функция не ждёт завершения процесса из-за ограничений ОС Windows
     */
    static void executeAsAdministrator(const AString& applicationFile,
                                      const AString& args = {},
                                      const APath& workingDirectory = {});
};


