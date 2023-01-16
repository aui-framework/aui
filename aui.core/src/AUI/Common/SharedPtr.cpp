// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
                assert(anotherSharedPtr.get() == pointingTo);
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