#include "ImageCompare.h"

#include <cmath>

namespace aui_remote::cli {

ImageDiff compareImages(const AImageView& a, const AImageView& b, int threshold) {
    ImageDiff result;
    if (a.size() != b.size()) {
        result.sizeMismatch = true;
        return result;
    }
    result.totalPixels = std::size_t(a.width()) * a.height();
    const float limit = float(threshold) / 255.f;
    for (unsigned y = 0; y < a.height(); ++y) {
        for (unsigned x = 0; x < a.width(); ++x) {
            const auto pa = a.get({ x, y });
            const auto pb = b.get({ x, y });
            if (std::abs(pa.r - pb.r) > limit || std::abs(pa.g - pb.g) > limit || std::abs(pa.b - pb.b) > limit ||
                std::abs(pa.a - pb.a) > limit) {
                ++result.differentPixels;
            }
        }
    }
    return result;
}

}   // namespace aui_remote::cli
