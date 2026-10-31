#pragma once

#include <AUI/IO/APath.h>
#include <AUI/Util/ARandom.h>

#include <gtest/gtest.h>

namespace aui_remote::test {

/**
 * @brief Empty temporary directory removed on destruction.
 */
class TempDir {
public:
    TempDir() {
        ARandom random;
        mPath = APath::getDefaultPath(APath::TEMP) / ("aui-remote-test-" + AString::number(random.nextInt() & 0xffffff));
        mPath.makeDirs();
    }
    ~TempDir() {
        try {
            mPath.removeFileRecursive();
        } catch (...) {
        }
    }

    [[nodiscard]] const APath& path() const noexcept { return mPath; }
    [[nodiscard]] APath operator/(const AString& name) const { return mPath / name; }

private:
    APath mPath;
};

}   // namespace aui_remote::test
