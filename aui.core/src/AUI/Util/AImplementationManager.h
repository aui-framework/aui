
#pragma once

#include <AUI/Traits/concepts.h>
#include <AUI/Common/SharedPtr.h>

/**
 * @brief Helps with picking implementation specified in template arguments which does not throw an exception in
 * tryAllUntilSuccess.
 * @ingroup core
 */
template<typename Base, aui::derived_from<Base>... Implementations>
struct AImplementationManager {
    template<aui::mapper<std::unique_ptr<Base>, std::unique_ptr<Base>> Callback>
    static std::unique_ptr<Base> tryAllUntilSuccess(Callback&& callback) {
        std::unique_ptr<Base> result;
        (false || ... || [&]() -> bool {
            try {
                auto i = std::make_unique<Implementations>();
                result = callback(std::move(i));
                return result != nullptr;
            } catch (...) {}
            return false;
        }());

        return result;
    }
};
