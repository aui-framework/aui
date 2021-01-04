//
// Created by alex2 on 03.01.2021.
//

#include "BackgroundImage.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Image/Drawables.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/ASS/AAssHelper.h>
#include <glm/gtc/matrix_transform.hpp>

void ass::decl::Declaration<ass::BackgroundImage>::renderFor(AView* view) {
    ass::BackgroundImage& info = view->getAssHelper()->state.backgroundUrl;
    if (info.url) {
        if (auto drawable = Drawables::get(*info.url)) {
            auto drawableDrawWrapper = [&](const glm::ivec2& size) {
                RenderHints::PushColor c;
                Render::inst().setColor(info.overlayColor.or_default(0xffffff_rgb));
                Render::inst().setRepeat(info.rep.or_default(REPEAT_NONE));
                drawable->draw(size);
                Render::inst().setRepeat(REPEAT_NONE);
            };

            if (info.sizing.or_default(Sizing::NONE) == Sizing::FIT) {
                drawableDrawWrapper(view->getSize());
            } else if (info.sizing.or_default(Sizing::NONE) == Sizing::FIT_PADDING) {
                RenderHints::PushMatrix m;
                Render::inst().setTransform(
                        glm::translate(glm::mat4(1.f),
                                       glm::vec3{view->getPadding().left, view->getPadding().top, 0.f}));
                drawableDrawWrapper(
                        view->getSize() - glm::ivec2{view->getPadding().horizontal(), view->getPadding().vertical()});
            } else {
                auto imageSize = glm::vec2(drawable->getSizeHint());
                if (drawable->isDpiDependent())
                    imageSize *= AWindow::current()->getDpiRatio();

                RenderHints::PushMatrix m;
                Render::inst().setTransform(
                        glm::translate(glm::mat4(1.f),
                                       glm::vec3(glm::ivec2((glm::vec2(view->getSize()) - imageSize) / 2.f), 0.f)));
                drawableDrawWrapper(imageSize);
            }
        }
    }
}


void ass::decl::Declaration<ass::BackgroundImage>::applyFor(AView* view) {
    view->getAssHelper()->state.backgroundUrl = mInfo;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundImage>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_IMAGE;
}


