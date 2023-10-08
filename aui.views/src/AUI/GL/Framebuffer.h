
// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
        glm::u32vec2 oversampledSize() const noexcept {
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

    private:
        uint32_t mHandle = 0;
        std::uint32_t mSupersamplingRatio = 1; 
        glm::u32vec2 mSize;
        AVector<_<IRenderTarget>> mAttachedTargets;
    };
}
