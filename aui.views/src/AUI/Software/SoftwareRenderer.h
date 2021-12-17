#pragma once


#include <AUI/Render/IRenderer.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Platform/SoftwareRenderingContext.h>

class SoftwareRenderer: public IRenderer {
private:
    SoftwareRenderingContext* mContext;
    bool mDrawingToStencil = false;
    enum {
        INCREASE = 1,
        DECREASE = -1
    } mDrawingStencilDirection;
    Blending mBlending = Blending::NORMAL;

public:
    /**
     * Draws a pixel onto the software framebuffer following the stencil and blending rules.
     * <dl>
     *   <dt><b>Assertions</b></dt>
     *   <dd><code>position</code> is inside the framebuffer.</dd>
     * </dl>
     * @param position position. An assertion is triggered if position is not inside the framebuffer.
     * @param color color.
     * @param blending blending. Optional. Cheaper. When set, the one set by the <code>setBlending</code> function is
     *        ignored.
     */
    inline void putPixel(const glm::ivec2& position, const AColor& color, std::optional<Blending> blending = std::nullopt) noexcept {
        assert(("context is null" && mContext != nullptr));
        auto actualBlending = blending ? *blending : mBlending;
        glm::uvec2 uposition(position);
        if (!glm::all(glm::lessThan(uposition, mContext->bitmapSize()))) return;

        if (mDrawingToStencil) {
            if (color.a > 0.5f) {
                mContext->stencil(position) += mDrawingStencilDirection;
            }
        } else {
            auto bufferStencilValue = mContext->stencil(position);
            if (bufferStencilValue == mStencilDepth)
            {
                switch (actualBlending) {
                    case Blending::NORMAL:
                        if (color.a >= 0.9999f) {
                            mContext->putPixel(uposition, glm::u8vec3(glm::vec3(color) * 255.f));
                        } else {
                            // blending
                            auto u8srcColor = mContext->getPixel(uposition);
                            auto srcColor = glm::vec3(u8srcColor.r, u8srcColor.g, u8srcColor.b);
                            mContext->putPixel(uposition, glm::u8vec3(glm::mix(srcColor, glm::vec3(color) * 255.f, color.a)));
                        }
                        break;

                    case Blending::ADDITIVE: {
                        auto src = glm::uvec3(glm::vec3(color) * 255.f);
                        auto dst = glm::uvec3(mContext->getPixel(uposition));
                        mContext->putPixel(uposition, (glm::min)(src + dst, glm::uvec3(255)));
                        break;
                    }
                    case Blending::INVERSE_DST: {
                        auto src = glm::vec3(color);
                        auto dst = glm::vec3(mContext->getPixel(uposition)) / 255.f;
                        mContext->putPixel(uposition, (glm::min)(glm::uvec3((src * (1.f - dst)) * 255.f), glm::uvec3(255)));
                        break;
                    }
                    case Blending::INVERSE_SRC:
                        auto src = glm::vec3(color);
                        auto dst = glm::vec3(mContext->getPixel(uposition)) / 255.f;
                        mContext->putPixel(uposition, (glm::min)(glm::uvec3(((1.f - src) * dst) * 255.f), glm::uvec3(255)));
                        break;
                }
            }
        }
    }
    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

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

    void pushMaskBefore() override;

    void pushMaskAfter() override;

    void popMaskBefore() override;

    void popMaskAfter() override;


protected:
    ITexture* createNewTexture() override;

};


