#pragma once

#include <AUI/Image/AImageView.h>

namespace aui_remote::cli {

struct ImageDiff {
    bool sizeMismatch = false;
    std::size_t totalPixels = 0;
    std::size_t differentPixels = 0;

    [[nodiscard]] double percent() const noexcept {
        return sizeMismatch ? 100.0 : totalPixels == 0 ? 0.0 : 100.0 * double(differentPixels) / double(totalPixels);
    }
};

/**
 * @brief Counts pixels that differ.
 * @param threshold a pixel is different if any of its channels differs by more than this (0..255)
 */
ImageDiff compareImages(const AImageView& a, const AImageView& b, int threshold = 0);

}   // namespace aui_remote::cli
