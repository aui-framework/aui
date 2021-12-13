//
// Created by Alex2772 on 12/5/2021.
//

#include <AUI/Traits/callables.h>
#include "SoftwareRenderer.h"

struct BrushHelper {
    SoftwareRenderer* renderer;
    int &x, &y;

    BrushHelper(SoftwareRenderer* renderer, int& x, int& y) : renderer(renderer), x(x), y(y) {}

    void operator()(const ASolidBrush& brush) {
        renderer->putPixel({x, y}, renderer->getColor() * brush.solidColor);
    }

    void operator()(const ATexturedBrush& brush) {
    }

    void operator()(const ALinearGradientBrush& brush) {
    }
};

struct RoundedRect {
    int radius;
    int radius2;\
    glm::ivec2 size;
    glm::ivec2 halfSize;
    glm::ivec2 transformedPosition;
    glm::ivec2 center;

    RoundedRect(int radius, const glm::ivec2& size, const glm::ivec2& transformedPosition):
        radius(radius),
        radius2(radius * radius),
        size(size),
        halfSize(size / 2),
        transformedPosition(transformedPosition),
        center(transformedPosition + halfSize)
    {

    }

    template<bool isAntialiasing>
    int test(const glm::ivec2& absed) const {
        int radius25 = radius2 * 25; // for antialiasing
        if (absed.x + radius >= halfSize.x && absed.y + radius >= halfSize.y) {
            auto circleCoordSpace = halfSize - absed - radius;
            // chamfer check
            if (circleCoordSpace.x + circleCoordSpace.y + radius < 0) {
                // circle check
                if constexpr (isAntialiasing) {
                    circleCoordSpace *= 5;
                    int accumulator = 0;
                    for (int oY = 0; oY < 5; ++oY) {
                        for (int oX = 0; oX < 5; ++oX) {
                            int fX = circleCoordSpace.x + oX;
                            int fY = circleCoordSpace.y + oY;
                            if (fX * fX + fY * fY < radius25) {
                                accumulator += 1;
                            }
                        }
                    }
                    return accumulator;
                }
                else {
                    return circleCoordSpace.x * circleCoordSpace.x + circleCoordSpace.y * circleCoordSpace.y > radius2;
                }
            }
        }
        return 25;
    }

    glm::ivec2 abs(const glm::ivec2& pos) {
        auto absed = pos - center;
        if (absed.x < 0) {
            absed.x = -absed.x;
        }
        else {
            absed.x += !(size.x % 2);
        }
        if (absed.y < 0) {
            absed.y = -absed.y;
        }
        else {
            absed.y += !(size.y % 2);
        }
        return absed;
    }
};


glm::mat4 SoftwareRenderer::getProjectionMatrix() const {
    return glm::mat4(1.f);
}

void SoftwareRenderer::drawRect(const ABrush& brush,
                                const glm::vec2& position,
                                const glm::vec2& size) {
    auto transformedPosition = glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f));
    auto end = transformedPosition + glm::ivec2(size);

    int x, y;

    auto sw = aui::lambda_overloaded {
            BrushHelper(this, x, y),
    };

    for (y = transformedPosition.y; y < end.y; ++y) {
        for (x = transformedPosition.x; x < end.x; ++x) {
            std::visit(sw, brush);
        }
    }
}

void SoftwareRenderer::drawRoundedRect(const ABrush& brush,
                                       const glm::vec2& position,
                                       const glm::vec2& size,
                                       float radius) {
    RoundedRect r(int(radius), glm::ivec2(size), glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f)));
    auto end = r.transformedPosition + r.size;

    int x, y;

    auto sw = aui::lambda_overloaded{
            BrushHelper(this, x, y),
    };

    for (y = r.transformedPosition.y; y < end.y; ++y) {
        for (x = r.transformedPosition.x; x < end.x; ++x) {
            if (r.test<false>(r.abs({x, y}))) {
                continue;
            }
            //std::visit(sw, brush);
        }
    }
}

void SoftwareRenderer::drawRoundedRectAntialiased(const ABrush& brush,
                                                  const glm::vec2& position,
                                                  const glm::vec2& size,
                                                  float radius) {

    RoundedRect r(int(radius), glm::ivec2(size), glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f)));
    auto end = r.transformedPosition + r.size;

    int x, y;

    auto sw = aui::lambda_overloaded{
            BrushHelper(this, x, y),
    };

    for (y = r.transformedPosition.y; y < end.y; ++y) {
        for (x = r.transformedPosition.x; x < end.x; ++x) {
            int accumulator = r.test<true>(r.abs({x, y}));
            if (accumulator != 0) {
                float alphaCopy = mColor.a;
                mColor.a *= accumulator;
                mColor.a /= 25;
                std::visit(sw, brush);
                mColor.a = alphaCopy;
            }
        }
    }
}

void SoftwareRenderer::drawRectBorder(const ABrush& brush,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      float lineWidth) {
    drawRect(brush, position, {size.x, lineWidth});
    drawRect(brush, position + glm::vec2{0, size.y - lineWidth}, { size.x, lineWidth });
    drawRect(brush, position + glm::vec2{ 0, lineWidth }, { lineWidth, size.y - 2 * lineWidth });
    drawRect(brush, position + glm::vec2{ size.x - lineWidth, lineWidth }, { lineWidth, size.y - 2 * lineWidth });
}

void SoftwareRenderer::drawRectBorder(const ABrush& brush,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      float radius,
                                      int borderWidth) {
    auto pos = glm::ivec2(mTransform * glm::vec4(position, 1.f, 1.f));
    RoundedRect outside(int(radius), glm::ivec2(size), pos);
    RoundedRect inside(int(radius), glm::ivec2(size) - glm::ivec2(borderWidth * 2), pos + glm::ivec2(borderWidth));
    auto end = outside.transformedPosition + outside.size;

    int x, y;

    auto sw = aui::lambda_overloaded{
            BrushHelper(this, x, y),
    };

    for (y = outside.transformedPosition.y; y < end.y; ++y) {
        for (x = outside.transformedPosition.x; x < end.x; ++x) {
            int accumulator = outside.test<true>(outside.abs({ x, y }));

            if (x - outside.transformedPosition.x >= borderWidth &&
                y - outside.transformedPosition.y >= borderWidth) {
                if (x < end.x - borderWidth &&
                    y < end.y - borderWidth) {
                    accumulator -= inside.test<true>(inside.abs({ x, y }));
                }
            }

            if (accumulator != 0) {
                float alphaCopy = mColor.a;
                mColor.a *= accumulator;
                mColor.a /= 25;
                std::visit(sw, brush);
                mColor.a = alphaCopy;
            }
        }
    }
}

glm::vec4 erf(glm::vec4 x) {
    glm::vec4 s = glm::sign(x), a = glm::abs(x);
    x = 1.0f + (0.278393f + (0.230389f + 0.078108f * (a * a)) * a) * a;
    x *= x;
    return s - s / (x * x);
}

void SoftwareRenderer::drawBoxShadow(const glm::vec2& position,
                                     const glm::vec2& size,
                                     float blurRadius,
                                     const AColor& color) {

    auto transformedPos = glm::vec2(mTransform * glm::vec4(position, 1.f, 1.f));
    float sigma = blurRadius / 2.f;
    glm::vec2 lower = transformedPos + size;
    glm::vec2 upper = transformedPos;
    auto finalColor = mColor * color;

    transformedPos -= blurRadius;
    glm::ivec2 iTransformedPos(transformedPos);
    auto eSize = size + blurRadius * 2.f;
    auto iSize = glm::ivec2(eSize);


    auto ePos = transformedPos;

    for (int y = 0; y < iSize.y; ++y) { 
        for (int x = 0; x < iSize.x; ++x) {
            glm::vec2 pass_uv = transformedPos + glm::vec2{x, y};
            glm::vec4 query = glm::vec4(pass_uv - glm::vec2(lower), pass_uv - glm::vec2(upper));
            glm::vec4 integral = 0.5f + 0.5f * erf(query * (glm::sqrt(0.5f) / sigma));
            float alpha = glm::clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0f, 1.0f);

            putPixel(iTransformedPos + glm::ivec2{ x, y }, { finalColor.r, finalColor.g, finalColor.b, finalColor.a * alpha });
        }
    }
}

void SoftwareRenderer::drawString(const glm::vec2& position,
                                  const AString& string,
                                  const AFontStyle& fs) {

}


void SoftwareRenderer::setBlending(Blending blending) {

}

class SoftwareTexture: public ITexture {
private:
    _<AImage> mImage;

public:
    void setImage(const _<AImage>& image) override {
        mImage = image;
    }
};

class SoftwarePrerenderedString: public IRenderer::IPrerenderedString {
public:
    void draw() override {

    }

    int getWidth() override {
        return 0;
    }

    int getHeight() override {
        return 0;
    }
};

class SoftwareMultiStringCanvas: public IRenderer::IMultiStringCanvas {
public:
    void addString(const glm::vec2& position,
                   const AString& text) override {

    }

    _<IRenderer::IPrerenderedString> build() override {
        return _<IRenderer::IPrerenderedString>();
    }

    ATextLayoutHelper makeTextLayoutHelper() override {
        return ATextLayoutHelper();
    }
};

_<IRenderer::IPrerenderedString> SoftwareRenderer::prerenderString(const glm::vec2& position,
                                                                   const AString& text,
                                                                   const AFontStyle& fs) {
    return _new<SoftwarePrerenderedString>();
}
ITexture* SoftwareRenderer::createNewTexture() {
    return new SoftwareTexture;
}

_<IRenderer::IMultiStringCanvas> SoftwareRenderer::newMultiStringCanvas(const AFontStyle style) {
    return _new<SoftwareMultiStringCanvas>();
}

void SoftwareRenderer::setWindow(ABaseWindow* window) {
    IRenderer::setWindow(window);
    if (auto context = dynamic_cast<SoftwareRenderingContext*>(window->getRenderingContext().get())) {
        mContext = context;
    } else {
        mContext = nullptr;
    }
}
