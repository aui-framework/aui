//
// Created by Alex2772 on 11/18/2021.
//

#include "IRenderer.h"

glm::mat4 IRenderer::getProjectionMatrix() const {
    return glm::ortho(0.f, static_cast<float>(mWindow->getWidth()), static_cast<float>(mWindow->getHeight()), 0.f);
}
