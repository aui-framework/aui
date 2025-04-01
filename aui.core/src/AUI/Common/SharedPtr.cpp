/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SharedPtr.h"
#include "SharedPtrTypes.h"
#include "AUI/Logging/ALogger.h"

#ifdef AUI_SHARED_PTR_FIND_INSTANCES

aui::impl::shared_ptr::InstancesDict& aui::impl::shared_ptr::instances() noexcept {
    static aui::impl::shared_ptr::InstancesDict r;
    return r;
}

void aui::impl::shared_ptr::printAllInstancesOf(void* ptrToSharedPtr) noexcept {
    static constexpr auto LOG_TAG = "AUI_SHARED_PTR_FIND_INSTANCES";
    auto& sharedPtr = *reinterpret_cast<_<void>*>(ptrToSharedPtr);
    void* pointingTo = sharedPtr.get();
    std::cout << "[" << LOG_TAG << "] shared_ptrs pointing to " << pointingTo << std::endl;
    std::unique_lock lock(instances().sync);
    if (auto it = instances().map.find(pointingTo); it != instances().map.end()) {
        if (it->second.empty()) {
            std::cout << "[" << LOG_TAG << "] <empty set>" << std::endl;
        } else {
            for (const auto& pSharedPtr : it->second) {
                auto& anotherSharedPtr = *reinterpret_cast<_<void>*>(pSharedPtr);
                AUI_ASSERT(anotherSharedPtr.get() == pointingTo);
                std::cout << "[" << LOG_TAG << "] "  << pSharedPtr;
                if (&sharedPtr == &anotherSharedPtr) {
                    std::cout << " (this)";
                }
                std::cout << " at:" << std::endl
                          << *anotherSharedPtr.mInstanceDescriber.stacktrace;
            }
        }
    } else {
        std::cout << "[" << LOG_TAG << "] <unknown ptr>" << std::endl;
    }
    std::flush(std::cout);
}

#endif