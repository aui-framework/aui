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