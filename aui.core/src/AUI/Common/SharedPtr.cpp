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