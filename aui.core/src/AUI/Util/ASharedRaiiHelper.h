#pragma once


#include <functional>
#include <utility>
#include <AUI/Common/AObject.h>

/**
 * @brief Shared completion marker for multithreaded callback-based future handling.
 * @ingroup core
 *
 * @details
 * ASharedRaiiHelper is intended to track the completion of multiple futures, exploiting the destruction
 * of AFuture::onSuccess or AFuture::onError callbacks when they called. This approach allows to easily integrate
 * ASharedRaiiHelper to the existing code.
 *
 * Note that the callback will be called even if onSuccess is not.
 *
 * @code{cpp}
 * auto marker = ASharedRaiiHelper::make([] {
 *     // will be called when all futures called onSuccess or destroyed
 * });
 * auto future = async {
 *     // hard work
 * }
 * //                      VVVVVV ASharedRaiiHelper should be captured!
 * future.onSuccess([this, marker]() {
 *     // handle the hard work
 * });
 *
 * // keep the future alive in async holder
 * mAsyncHolder << future;
 *
 * @endcode
 */
class ASharedRaiiHelper {
public:
    static _<ASharedRaiiHelper> make(std::function<void()> callback) {
        return aui::ptr::manage(new ASharedRaiiHelper(std::move(callback)));
    }

    ~ASharedRaiiHelper() {
        callback();
    }

private:
    std::function<void()> callback;

    explicit ASharedRaiiHelper(std::function<void()> callback) : callback(std::move(callback)) {}
};


