#pragma once


#include <AUI/GL/Shader.h>
#include <AUI/GL/Vao.h>
#include "IRenderer.h"

class OpenGLRenderer: public IRenderer {
private:
    GL::Shader mSolidShader;
    GL::Shader mGradientShader;
    GL::Shader mRoundedSolidShader;
    GL::Shader mRoundedSolidShaderAntialiased;
    GL::Shader mRoundedGradientShaderAntialiased;
    GL::Shader mSolidTransformShader;
    GL::Shader mBoxShadowShader;
    GL::Shader mTexturedShader;
    GL::Shader mSymbolShader;
    GL::Shader mSymbolShaderSubPixel;
    GL::Vao mTempVao;


    glm::mat4 getProjectionMatrix() const;
    AVector<glm::vec3> getVerticesForRect(const glm::vec2& position,
                                          const glm::vec2& size);

    void uploadToShaderCommon();

    void endDraw(const ABrush& brush);

protected:
    ITexture* createNewTexture() override;

public:
    OpenGLRenderer();
    ~OpenGLRenderer() override = default;

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
                    const FontStyle& fs) override;

    _<IPrerenderedString> prerenderString(const glm::vec2& position, const AString& text, const FontStyle& fs) override;

    void drawRectImpl(const glm::vec2& position, const glm::vec2& size);
};


