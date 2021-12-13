#pragma once


#include <AUI/Render/IRenderer.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Platform/SoftwareRenderingContext.h>

class SoftwareRenderer: public IRenderer {
private:
    SoftwareRenderingContext* mContext;

public:

    inline void putPixel(const glm::ivec2& position, const AColor& color) {
        assert(("context is null" && mContext != nullptr));
        glm::uvec2 uposition(position);

        if (color.a >= 0.9999f) {
            mContext->putPixel(uposition, glm::u8vec3(glm::vec3(color) * 255.f));
        } else {
            // blending
            auto u8srcColor = mContext->getPixel(uposition);
            auto srcColor = glm::vec3(u8srcColor.r, u8srcColor.g, u8srcColor.b);
            mContext->putPixel(uposition, glm::u8vec3(glm::mix(srcColor, glm::vec3(color) * 255.f, color.a)));
        }
    }
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

    void setWindow(ABaseWindow* window) override;

    glm::mat4 getProjectionMatrix() const override;


protected:
    ITexture* createNewTexture() override;

};


