//
// Created by Alex2772 on 12/5/2021.
//

#include <AUI/Traits/callables.h>
#include "SoftwareRenderer.h"
#include <AUI/Platform/SoftwareRenderingContext.h>

void SoftwareRenderer::putPixel(const glm::ivec2& position, const AColor& color) {
    image().setPixelAt(position.x, position.y, glm::ivec4(color * 255.f));
}

void SoftwareRenderer::drawRect(const ABrush& brush,
                                const glm::vec2& position,
                                const glm::vec2& size) {
    auto end = position + size;
    for (int y = position.y; y < end.y; ++y) {
        for (int x = position.x; x < end.x; ++x) {
            std::visit(aui::lambda_overloaded {
                [&](const ASolidBrush& brush) {
                    putPixel({x, y}, brush.solidColor);
                },
                [&](const ATexturedBrush& brush) {

                },
                [&](const ALinearGradientBrush& brush) {

                },
            }, brush);
        }
    }
}

void SoftwareRenderer::drawRoundedRect(const ABrush& brush,
                                       const glm::vec2& position,
                                       const glm::vec2& size,
                                       float radius) {

}

void SoftwareRenderer::drawRoundedRectAntialiased(const ABrush& brush,
                                                  const glm::vec2& position,
                                                  const glm::vec2& size,
                                                  float radius) {

}

void SoftwareRenderer::drawRectBorder(const ABrush& brush,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      float lineWidth) {

}

void SoftwareRenderer::drawRectBorder(const ABrush& brush,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      float radius,
                                 int borderWidth) {

}

void SoftwareRenderer::drawBoxShadow(const glm::vec2& position,
                                     const glm::vec2& size,
                                     float blurRadius,
                                     const AColor& color) {

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
    if (auto w = dynamic_cast<SoftwareRenderingContext*>(window->getRenderingContext().get())) {

    }
}
