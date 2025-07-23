#pragma once

#include <vector>

namespace aui::impl {
template<class T>
void reserveVector(std::vector<T>& vec, size_t count) {
    if (vec.size() < count) {
        vec.resize(count);
    }
}
}
