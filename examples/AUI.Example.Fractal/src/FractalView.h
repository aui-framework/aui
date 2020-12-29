#pragma once


#include <AUI/View/AView.h>
#include <AUI/GL/Shader.h>
#include <AUI/GL/Texture2D.h>

class FractalView: public AView {
private:
    GL::Shader mShader;
    _<GL::Texture2D> mTexture;
    glm::dmat4 mTransform;

    float mAspectRatio;

    void handleMatrixUpdated();

public:
    FractalView();
    void render() override;

    void reset();

    void setIterations(unsigned it);

    void onKeyDown(AInput::Key key) override;

    void onKeyRepeat(AInput::Key key) override;

    void onMouseWheel(glm::ivec2 pos, int delta) override;

    void setSize(int width, int height) override;

    GL::Shader& getShader() {
        return mShader;
    }

    const _<GL::Texture2D>& getTexture() const {
        return mTexture;
    }

    glm::dvec2 getPlotPosition() const;
    double getPlotScale() const;

    void setPlotPositionAndScale(glm::dvec2 position, double scale);

signals:

    emits<glm::dvec2, double> centerPosChanged;
};

