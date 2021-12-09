#pragma once


#include <AUI/Render/IRenderer.h>

class SoftwareRenderer: public IRenderer {
public:
    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle style) override;

    void drawRect(const ABrush& brush,
                  const glm::vec2& position,
                  const glm::vec2& size) override;

    void drawRoundedRect(const ABrush& brush,
                         const glm::vec2& position,
                         const glm::vec2& size,
                         float radius) override;

    void drawRoundedRectAntialiased(const ABrush& brush,
                                    const glm::vec2& position,
                                    const glm::vec2& size,
                                    float radius) override;

    void drawRectBorder(const ABrush& brush,
                        const glm::vec2& position,
                        const glm::vec2& size,
                        float lineWidth) override;

    void drawRectBorder(const ABrush& brush,
                        const glm::vec2& position,
                        const glm::vec2& size,
                        float radius,
                        int borderWidth) override;

    void drawBoxShadow(const glm::vec2& position,
                       const glm::vec2& size,
                       float blurRadius,
                       const AColor& color) override;

    void drawString(const glm::vec2& position,
                    const AString& string,
                    const AFontStyle& fs) override;

    _<IPrerenderedString> prerenderString(const glm::vec2& position,
                                          const AString& text,
                                          const AFontStyle& fs) override;

    void setBlending(Blending blending) override;

protected:
    ITexture* createNewTexture() override;
};


