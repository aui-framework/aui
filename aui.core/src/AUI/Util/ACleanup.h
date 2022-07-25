#pragma once

#include <functional>
#include <AUI/Common/AObject.h>
#include <AUI/Common/AQueue.h>


/**
 * @brief Cleanup helper.
 * @ingroup core
 */
class API_AUI_CORE ACleanup {
    friend void afterEntryCleanup();
public:
    using Callback = std::function<void()>;

    /**
     * @param callback callback that called after AUI_ENTRY but before native entry point exit.
     */
    static void afterEntry(Callback callback) {
        std::unique_lock lock(inst().mSync);
        inst().mCallbacks << std::move(callback);
    }

private:
    static ACleanup& inst();

    void afterEntryPerform();

    AMutex mSync;
    AQueue<Callback> mCallbacks;
};


