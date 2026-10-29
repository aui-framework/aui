#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <AUI/Common/AOptional.h>

namespace aui_remote {

/**
 * @brief Maps a point of the viewport (local view pixels) to the logical pixels of the controlled application window.
 * @details
 * The frame is drawn with the aspect ratio of the window preserved and centered, so when the viewport has a different
 * aspect ratio there are letterbox bars. Since the viewport is positioned by ARulerArea (which only moves it, there is
 * no zoom), local coordinates already account for the ruler offset; the scale accounts for the viewport being bigger or
 * smaller than the window. The downscale of the frame doesn't matter, as the frame always covers the whole window.
 */
struct ViewportMapping {
    glm::vec2 viewportSize{};
    glm::vec2 windowSize{};

    /**
     * @return Where the frame is drawn inside the viewport.
     */
    struct Rect {
        glm::vec2 offset{};
        glm::vec2 size{};
    };

    [[nodiscard]] bool valid() const noexcept {
        return windowSize.x > 0 && windowSize.y > 0 && viewportSize.x > 0 && viewportSize.y > 0;
    }

    /**
     * @return Viewport pixels per window logical pixel.
     */
    [[nodiscard]] float scale() const noexcept {
        if (!valid()) {
            return 1.f;
        }
        return std::min(viewportSize.x / windowSize.x, viewportSize.y / windowSize.y);
    }

    [[nodiscard]] Rect content() const noexcept {
        const auto size = windowSize * scale();
        return { (viewportSize - size) / 2.f, size };
    }

    /**
     * @return Window logical position, or nullopt if the point is on a letterbox bar.
     */
    [[nodiscard]] AOptional<glm::vec2> toWindow(glm::vec2 point) const noexcept {
        if (!valid()) {
            return std::nullopt;
        }
        const auto rect = content();
        const auto local = point - rect.offset;
        if (local.x < 0 || local.y < 0 || local.x >= rect.size.x || local.y >= rect.size.y) {
            return std::nullopt;
        }
        return local / scale();
    }

    /**
     * @return Viewport position of a point of the window; the inverse of toWindow().
     */
    [[nodiscard]] glm::vec2 toViewport(glm::vec2 windowPoint) const noexcept {
        return content().offset + windowPoint * scale();
    }

    /**
     * @return Window logical position; points outside are clamped to the window edge (for a drag that left the frame).
     */
    [[nodiscard]] glm::vec2 toWindowClamped(glm::vec2 point) const noexcept {
        if (!valid()) {
            return { 0, 0 };
        }
        const auto local = (point - content().offset) / scale();
        return glm::clamp(local, glm::vec2(0.f), glm::max(windowSize - 1.f, glm::vec2(0.f)));
    }
};

}   // namespace aui_remote
