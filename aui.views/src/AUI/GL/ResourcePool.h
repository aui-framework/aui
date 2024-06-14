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