/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 3.12.2020.
//

#pragma once

#include <AUI/Render/Render.h>
#include "IDeclaration.h"

namespace ass {

    /**
     * @brief Controls background texture cropping.
     * @ingroup ass
     */
    struct BackgroundCropping {
        unset_wrap<glm::vec2> offset;
        unset_wrap<glm::vec2> size;

        BackgroundCropping() noexcept = default;
        BackgroundCropping(const BackgroundCropping&) noexcept = default;
        BackgroundCropping(const glm::vec2& offset, const unset_wrap<glm::vec2>& size) noexcept:
            offset(offset), size(size) {}

        static const BackgroundCropping H4_1;
        static const BackgroundCropping H4_2;
        static const BackgroundCropping H4_3;
        static const BackgroundCropping H4_4;
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BackgroundCropping>: IDeclarationBase {
        private:
            BackgroundCropping mInfo;

        public:
            Declaration(const BackgroundCropping& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            const BackgroundCropping& value() {
                return mInfo;
            }
        };

    }
}

inline const ass::BackgroundCropping ass::BackgroundCropping::H4_1 = { glm::vec2{0.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_2 = { glm::vec2{1.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_3 = { glm::vec2{2.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_4 = { glm::vec2{3.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };