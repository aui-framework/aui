// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <AUI/GL/gl.h>
#include "AUI/Common/AQueue.h"

namespace gl {

    enum class ResourceKind {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE,
    };

    namespace impl {
        template<ResourceKind T>
        struct resource;

        struct resource_basic_buffer {
            static GLuint gen() noexcept {
                GLuint b = 0;
                glGenBuffers(1, &b);
                return b;
            }
            static void del(GLuint b) noexcept {
                glDeleteBuffers(1, &b);
            }
        };

        template<> struct resource<ResourceKind::VERTEX_BUFFER>: resource_basic_buffer {};
        template<> struct resource<ResourceKind::INDEX_BUFFER>: resource_basic_buffer {};

        template<> struct resource<ResourceKind::TEXTURE> {
            static GLuint gen() noexcept {
                GLuint b = 0;
                glGenTextures(1, &b);
                return b;
            }
            static void del(GLuint b) noexcept {
                glDeleteTextures(1, &b);
            }
        };
    }

    template<ResourceKind T>
    class ResourcePool {
    public:
        static GLuint get() {
            return inst().mObjects.popOrGenerate([] {
                return gl::impl::resource<T>::gen();
            });
        }
        static void put(GLuint obj) {
            inst().mObjects.push(obj);
        }

        ~ResourcePool() {
            while (!mObjects.empty()) {
                gl::impl::resource<T>::del(mObjects.front());
                mObjects.pop();
            }
        }

    private:
        AQueue<GLuint> mObjects;

        static ResourcePool& inst() noexcept {
            static ResourcePool r;
            return r;
        }
    };
}