
/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Traits/values.h"
#include "glm/fwd.hpp"
#include <AUI/Common/AString.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include <AUI/Views.h>
#include <AUI/GL/gl.h>

namespace gl {
    class API_AUI_VIEWS Framebuffer: public aui::noncopyable {
    public:
        
#if AUI_PLATFORM_IOS
        static constexpr auto DEFAULT_FB = 1;
#else
        static constexpr auto DEFAULT_FB = 0;
#endif

        class IRenderTarget {
        friend class Framebuffer;
        public:
            virtual ~IRenderTarget() = default;

        protected:
            virtual void onFramebufferResize(glm::u32vec2 size) = 0;
            virtual void attach(Framebuffer& to, GLenum attachmentType) = 0;
        };

        Framebuffer();
        Framebuffer(Framebuffer&& rhs) noexcept: mHandle(rhs.mHandle),
                                                 mSize(rhs.mSize),
                                                 mAttachedTargets(std::move(rhs.mAttachedTargets)),
                                                 mSupersamplingRatio(rhs.mSupersamplingRatio) {
            rhs.mHandle = 0;            
        }
        virtual ~Framebuffer();

        void bind();
        void bindForRead();
        void bindForWrite();
        static void unbind();
        void resize(glm::u32vec2 newSize);

        [[nodiscard]]
        std::uint32_t supersamlingRatio() const noexcept {
            return mSupersamplingRatio;
        }

        void setSupersamplingRatio(std::uint32_t ratio) noexcept {
            mSupersamplingRatio = ratio;
        }

        [[nodiscard]]
        glm::u32vec2 supersampledSize() const noexcept {
            return mSize * mSupersamplingRatio;
        }

        [[nodiscard]]
        glm::u32vec2 size() const noexcept {
            return mSize;
        }

        operator bool() const {
            return mHandle;
        }
        uint32_t getHandle() const {
            return mHandle;
        }

        void attach(_<IRenderTarget> renderTarget, GLenum attachmentType /* = GL_COLOR_ATTACHEMNT0 */) {
            renderTarget->attach(*this, attachmentType);
            unbind();
            mAttachedTargets << std::move(renderTarget);
        }

        Framebuffer& operator=(Framebuffer&& rhs) noexcept {
            if (mHandle == rhs.mHandle) {
                return *this;
            }
            std::swap(mHandle, rhs.mHandle);
            mSize = rhs.mSize;
            mSupersamplingRatio = rhs.mSupersamplingRatio;
            mAttachedTargets = std::move(rhs.mAttachedTargets);
            return *this;
        }

        static gl::Framebuffer* current();

    private:
        uint32_t mHandle = 0;
        std::uint32_t mSupersamplingRatio = 1; 
        glm::u32vec2 mSize{0, 0};
        AVector<_<IRenderTarget>> mAttachedTargets;
    };
}
