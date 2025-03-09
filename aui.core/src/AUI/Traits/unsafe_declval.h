#pragma once

#include <type_traits>

namespace aui {
namespace detail {
/**
 * @brief This function serves as a link-time assert. If linker requires it, then `unsafe_declval()` is used at runtime.
 */
void report_if_you_see_link_error_with_this_function() noexcept;
}   // namespace detail

/**
 * @brief For returning non default constructible types. Do NOT use at runtime!
 * @details
 * GCCs std::declval may not be used in potentially evaluated contexts, so we reinvent it.
 */
template <class T>
constexpr T unsafe_declval() noexcept {
    detail::report_if_you_see_link_error_with_this_function();

    typename std::remove_reference<T>::type* ptr = nullptr;
    ptr += 42;   // suppresses 'null pointer dereference' warnings
    return static_cast<T>(*ptr);
}
}   // namespace aui