#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Platform/AProcess.h>

class AStdOutputRecorder: public AObject {
public:
    AStdOutputRecorder(const _<AChildProcess>& process) {
        connect(process->stdOut, [&](const AByteBuffer& buffer) {
            mStdout << buffer;
        });
    }

    [[nodiscard]]
    const AByteBuffer& stdoutBuffer() const {
        return mStdout;
    }

private:
    AByteBuffer mStdout;
};