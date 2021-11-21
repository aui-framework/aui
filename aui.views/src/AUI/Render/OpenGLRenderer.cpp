//
// Created by Alex2772 on 11/19/2021.
//

#include "OpenGLRenderer.h"
#include "ShaderUniforms.h"
#include "Render.h"
#include <AUI/Traits/callables.h>


class OpenGLTexture2D: public ITexture {
private:
    GL::Texture2D mTexture;

public:
    void setImage(const _<AImage>& image) override {
        mTexture.tex2D(image);
    }

    void bind() {
        mTexture.bind();
    }
};


template<typename Brush>
struct UnsupportedBrushHelper {
    void operator()(const Brush& brush) const {
        assert(("this brush is unsupported"));
    }
};

struct GradientShaderHelper {
    GL::Shader& shader;

    GradientShaderHelper(GL::Shader& shader) : shader(shader) {}

    void operator()(const ALinearGradientBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor());
        shader.set(aui::ShaderUniforms::COLOR_TL, brush.topLeftColor);
        shader.set(aui::ShaderUniforms::COLOR_TR, brush.topRightColor);
        shader.set(aui::ShaderUniforms::COLOR_BL, brush.bottomLeftColor);
        shader.set(aui::ShaderUniforms::COLOR_BR, brush.bottomRightColor);
    }
};

struct SolidShaderHelper {
    GL::Shader& shader;

    SolidShaderHelper(GL::Shader& shader) : shader(shader) {}

    void operator()(const ASolidBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor() * brush.solidColor);
    }
};

struct TexturedShaderHelper {
    GL::Shader& shader;
    GL::Vao& tempVao;

    TexturedShaderHelper(GL::Shader& shader, GL::Vao& tempVao) : shader(shader), tempVao(tempVao) {}

    void operator()(const ATexturedBrush& brush) const {
        shader.use();
        switch (brush.imageRendering) {
            case ImageRendering::PIXELATED:
                GL::Texture2D::setupNearest();
                break;
            case ImageRendering::SMOOTH:
                GL::Texture2D::setupLinear();
                break;
        }
        glEnableVertexAttribArray(2);
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor());
        glm::vec2 uv1 = brush.uv1 ? *brush.uv1 : glm::vec2{0, 0};
        glm::vec2 uv2 = brush.uv2 ? *brush.uv2 : glm::vec2{1, 1};
        tempVao.insert(2, {
                glm::vec2{uv1.x, uv2.y},
                glm::vec2{uv2.x, uv2.y},
                glm::vec2{uv1.x, uv1.y},
                glm::vec2{uv2.x, uv1.y},
        });
        _cast<OpenGLTexture2D>(brush.texture)->bind();
    }
};


OpenGLRenderer::OpenGLRenderer() {
    mSolidShader.load(
            "attribute vec3 pos;"
            "void main(void) {gl_Position = vec4(pos, 1);}",
            "uniform vec4 color;"
            "void main(void) {gl_FragColor = color;}");
    mBoxShadowShader.load(
            "attribute vec3 pos;"
            "uniform mat4 transform;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = transform * vec4(pos, 1); pass_uv = pos.xy;}",
            "varying vec2 pass_uv;"
            "uniform vec4 color;"
            "uniform vec2 lower;"
            "uniform vec2 upper;"
            "uniform float sigma;"
            "vec4 erf(vec4 x) {"
            "vec4 s = sign(x), a = abs(x);"
            "x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;"
            "x *= x;"
            "return s - s / (x * x);"
            "}"
            "void main(void) {"
            "gl_FragColor = color;"

            "vec4 query = vec4(pass_uv - vec2(lower), pass_uv - vec2(upper));"
            "vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));"
            "gl_FragColor.a *= clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0);"
            //"gl_FragColor.a = query.x + query.y;"
            "}");
    /*

    if (glewGetExtension("ARB_multisample")) {
        mRoundedSolidShader.load(
                "attribute vec3 pos;"
                "attribute vec2 uv;"
                "varying vec2 pass_uv;"
                "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                "uniform vec2 size;"
                "varying vec2 pass_uv;"
                "void main(void) {"
                "vec2 tmp = abs(pass_uv);"
                "if ((tmp.x - 1.0) * (size.y) / (-size.x) < tmp.y - (1.0 - size.y) &&"
                "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) > 1.0) discard;"
                "}");
    } else */ {
        // without antialiasing rounded borders look poorly. fix it
        mRoundedSolidShader.load(
                "attribute vec3 pos;"
                "attribute vec2 uv;"
                "varying vec2 pass_uv;"
                "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                "uniform vec2 size;"
                "varying vec2 pass_uv;"
                "void main(void) {"
                "vec2 tmp = abs(pass_uv);"
                "if ((tmp.x - 1.0) * (size.y) / (-size.x) < tmp.y - (1.0 - size.y) &&"
                "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) > 1.0) discard;"
                "}");
        auto produceRoundedAntialiasedShader = [](GL::Shader& shader, const AString& uniforms, const AString& color) {
            shader.load(
                    "attribute vec3 pos;"
                    "attribute vec2 uv;"
                    "attribute vec2 outer_to_inner;"
                    "varying vec2 pass_uv;"
                    "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                    "uniform vec2 outerSize;"
                    "uniform vec2 innerSize;"
                    "uniform vec2 innerTexelSize;"
                    "uniform vec2 outerTexelSize;"
                    "uniform vec2 outer_to_inner;"
                    + uniforms +
                    "uniform vec4 color;"
                    "varying vec2 pass_uv;"
                    "bool is_outside(vec2 tmp, vec2 size) {"
                    "if (tmp.x >= 1.0 || tmp.y >= 1.0) return true;"
                    "return (tmp.x - 1.0) * (size.y) / (-size.x) <= tmp.y - (1.0 - size.y) &&"
                    "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                    "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) >= 1.0;"
                    "}"
                    "void main(void) {"
                    "vec2 outer_uv = abs(pass_uv);"
                    "vec2 inner_uv = outer_uv * outer_to_inner;"
                    "float alpha = 1.f;"
                    "ivec2 i;"
                    "for (i.x = -2; i.x <= 2; ++i.x) {"
                    "for (i.y = -2; i.y <= 2; ++i.y) {"
                    "alpha -= (is_outside(inner_uv + innerTexelSize * vec2(i), innerSize)"
                    " == "
                    "is_outside(outer_uv + outerTexelSize * vec2(i), outerSize)"
                    ") ? (1.0 / 25.0) : 0.0;"
                    "}"
                    "}"
                    + color +
                    "gl_FragColor = vec4(fcolor.rgb, fcolor.a * alpha);"
                    "}", { "pos", "uv" });
        };
        produceRoundedAntialiasedShader(mRoundedSolidShaderAntialiased,
                                        {},
                                        "vec4 fcolor = color;");

        produceRoundedAntialiasedShader(mRoundedGradientShaderAntialiased,
                                        "uniform vec4 color_tl;"
                                        "uniform vec4 color_tr;"
                                        "uniform vec4 color_bl;"
                                        "uniform vec4 color_br;",
                                        "vec4 fcolor = mix(mix(color_tl, color_tr, pass_uv.x), mix(color_bl, color_br, pass_uv.x), pass_uv.y) * color;");
    }

    mSolidTransformShader.load(
            "attribute vec3 pos;"
            "uniform mat4 transform;"
            "void main(void) {gl_Position = transform * vec4(pos, 1);}",
            "uniform vec4 color;"
            "void main(void) {gl_FragColor = color;}");

    mTexturedShader.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv;}",
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_FragColor = texture2D(tex, pass_uv) * color; if (gl_FragColor.a < 0.01) discard;}",
            {"pos", "", "uv"});

    mGradientShader.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv;}",
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "uniform vec4 color_tl;"
            "uniform vec4 color_tr;"
            "uniform vec4 color_bl;"
            "uniform vec4 color_br;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_FragColor = mix(mix(color_tl, color_tr, pass_uv.x), mix(color_bl, color_br, pass_uv.x), pass_uv.y) * color;}",
            {"pos", "uv"});

    mSymbolShader.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "uniform mat4 mat;"

            "void main(void) {gl_Position = mat * vec4(pos, 1); pass_uv = uv;}",
            "varying vec2 pass_uv;"
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "void main(void) {float sample = pow(texture2D(tex, pass_uv).r, 1.0 / 1.2); gl_FragColor = vec4(color.rgb, color.a * sample); if (gl_FragColor.a < 0.01) discard;}",
            {"pos", "uv"});

    mSymbolShaderSubPixel.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "uniform mat4 mat;"

            "void main(void) {gl_Position = mat * vec4(pos, 1); pass_uv = uv;}",
            "varying vec2 pass_uv;"
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "void main(void) {vec3 sample = pow(texture2D(tex, pass_uv).rgb, vec3(1.0 / 1.2)); gl_FragColor = vec4(sample * color.rgb * color.a, 1);}",
            {"pos", "uv"});

    mTempVao.bind();

    mTempVao.insert(1, {
                            {0, 1},
                            {1, 1},
                            {0, 0},
                            {1, 0}
                    }
    );
}

void OpenGLRenderer::uploadToShaderCommon() {
    GL::Shader::currentShader()->set(aui::ShaderUniforms::TRANSFORM, mTransform);
}

AVector<glm::vec3> OpenGLRenderer::getVerticesForRect(const glm::vec2& position, const glm::vec2& size)
{
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    return
            {
                    glm::vec3(mTransform * glm::vec4{ x, h, 1, 1 }),
                    glm::vec3(mTransform * glm::vec4{ w, h, 1, 1 }),
                    glm::vec3(mTransform * glm::vec4{ x, y, 1, 1 }),
                    glm::vec3(mTransform * glm::vec4{ w, y, 1, 1 }),
            };
}
void OpenGLRenderer::drawRect(const ABrush& brush, const glm::vec2& position, const glm::vec2& size) {
    std::visit(aui::lambda_overloaded {
        GradientShaderHelper(mGradientShader),
        TexturedShaderHelper(mTexturedShader, mTempVao),
        SolidShaderHelper(mSolidShader),
    }, brush);
    uploadToShaderCommon();

    drawRectImpl(position, size);

    endDraw(brush);
}

void OpenGLRenderer::drawRectImpl(const glm::vec2& position, const glm::vec2& size) {
    mTempVao.bind();

    mTempVao.insert(0, getVerticesForRect(position, size));

    mTempVao.indices({0, 1, 2, 2, 1, 3 });
    mTempVao.draw();
}

void OpenGLRenderer::drawRoundedRect(const ABrush& brush,
                                     const glm::vec2& position,
                                     const glm::vec2& size,
                                     float radius) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mRoundedSolidShader),
    }, brush);
    uploadToShaderCommon();

    mRoundedSolidShader.use();
    mRoundedSolidShader.set(aui::ShaderUniforms::SIZE, 2.f * radius / size);
    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawRoundedRectAntialiased(const ABrush& brush,
                                                const glm::vec2& position,
                                                const glm::vec2& size,
                                                float radius) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mRoundedGradientShaderAntialiased),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mRoundedSolidShaderAntialiased),
    }, brush);

    uploadToShaderCommon();

    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    GL::Shader::currentShader()->set(aui::ShaderUniforms::INNER_SIZE, glm::vec2{9999});
    GL::Shader::currentShader()->set(aui::ShaderUniforms::INNER_TEXEL_SIZE, glm::vec2{0, 0});
    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_TEXEL_SIZE, 2.f / 5.f / size);
    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, glm::vec2{0});

    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawRectBorder(const ABrush& brush,
                                    const glm::vec2& position,
                                    const glm::vec2& size,
                                    float lineWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mSolidShader),
    }, brush);
    uploadToShaderCommon();
    mTempVao.bind();

    //rect.insert(0, getVerticesForRect(x + 0.25f + lineWidth * 0.5f, y + 0.25f + lineWidth * 0.5f, width - (0.25f + lineWidth * 0.5f), height - (0.75f + lineWidth * 0.5f)));

    const float lineDelta = 0.25f + lineWidth / 2.f;
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    mTempVao.insert(0,
                    AVector<glm::vec3>{
                            glm::vec3(mTransform * glm::vec4{ x + lineWidth, y + lineDelta, 1, 1 }),
                            glm::vec3(mTransform * glm::vec4{ w,             y + lineDelta, 1, 1 }),

                            glm::vec3(mTransform * glm::vec4{ w - lineDelta, y + lineWidth, 1, 1 }),
                            glm::vec3(mTransform * glm::vec4{ w - lineDelta, h            , 1, 1 }),

                            glm::vec3(mTransform * glm::vec4{ w - lineWidth, h - lineDelta - 0.15f, 1, 1 }),
                            glm::vec3(mTransform * glm::vec4{ x            , h - lineDelta - 0.15f, 1, 1 }),

                            glm::vec3(mTransform * glm::vec4{ x + lineDelta, h - lineWidth - 0.15f, 1, 1 }),
                            glm::vec3(mTransform * glm::vec4{ x + lineDelta, y            , 1, 1 }),
                    });

    mTempVao.indices({ 0, 1, 2, 3, 4, 5, 6, 7});
    glLineWidth(lineWidth);
    mTempVao.draw(GL_LINES);
    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawRectBorder(const ABrush& brush,
                                    const glm::vec2& position,
                                    const glm::vec2& size,
                                    float radius,
                                    int borderWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mRoundedSolidShaderAntialiased),
    }, brush);

    glm::vec2 innerSize = { size.x - borderWidth * 2,
                            size.y - borderWidth * 2 };

    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    GL::Shader::currentShader()->set(aui::ShaderUniforms::INNER_SIZE, 2.f * (radius - borderWidth) / innerSize);
    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, size / innerSize);

    GL::Shader::currentShader()->set(aui::ShaderUniforms::INNER_TEXEL_SIZE, 2.f / 5.f / innerSize);
    GL::Shader::currentShader()->set(aui::ShaderUniforms::OUTER_TEXEL_SIZE, 2.f / 5.f / size);

    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawBoxShadow(const glm::vec2& position,
                                   const glm::vec2& size,
                                   float blurRadius,
                                   const AColor& color) {
    mBoxShadowShader.use();
    mBoxShadowShader.set(aui::ShaderUniforms::SIGMA, blurRadius / 2.f);
    mBoxShadowShader.set(aui::ShaderUniforms::LOWER, position + size);
    mBoxShadowShader.set(aui::ShaderUniforms::UPPER, position);
    mBoxShadowShader.set(aui::ShaderUniforms::TRANSFORM, mTransform);
    mBoxShadowShader.set(aui::ShaderUniforms::COLOR, mColor * color);

    mTempVao.bind();

    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    x -= blurRadius;
    y -= blurRadius;
    w += blurRadius;
    h += blurRadius;

    mTempVao.insert(0, AVector<glm::vec2>{
            glm::vec2{ x, h },
            glm::vec2{ w, h },
            glm::vec2{ x, y },
            glm::vec2{ w, y },
    });

    mTempVao.indices({ 0, 1, 2, 2, 1, 3 });
    mTempVao.draw();
}

void OpenGLRenderer::drawString(const glm::vec2& position,
                                const AString& string,
                                const FontStyle& fs) {
    prerenderString(position, string, fs)->draw();
}


void OpenGLRenderer::endDraw(const ABrush& brush) {
    if (std::holds_alternative<ATexturedBrush>(brush)) {
        glDisableVertexAttribArray(2);
    }
}


class OpenGLPrerenderedString: public IRenderer::IPrerenderedString {

public:
    OpenGLPrerenderedString() {}

    void draw() override {

    }

    int getWidth() override {
        return 0;
    }
};

_<IRenderer::IPrerenderedString>
OpenGLRenderer::prerenderString(const glm::vec2& position, const AString& text, const FontStyle& fs) {

    return _new<OpenGLPrerenderedString>();
}

ITexture* OpenGLRenderer::createNewTexture() {
    return new OpenGLTexture2D;
}
