#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/GL/gl.h>
#include <glm/glm.hpp>
#include "ResourcePool.h"

namespace GL {
    namespace detail {
        template<gl::ResourceKind T>
        class API_AUI_VIEWS VboImpl {
        protected:
            GLuint mHandle;

        public:
            VboImpl();
            VboImpl(const VboImpl&) = delete;

            VboImpl(VboImpl&& v): mHandle(v.mHandle) {
                v.mHandle = 0;
            }

            ~VboImpl();

        };
    }


    class API_AUI_VIEWS VertexBuffer: public detail::VboImpl<gl::ResourceKind::VERTEX_BUFFER> {
    private:
        void insert(const char* data, size_t length);

    public:
        void bind();


        template<typename T>
        void set(const T* data, size_t count) {
            static_assert(std::is_standard_layout_v<T>, "you may use only standard layout types in GL::VertexBuffer::set");
            insert(reinterpret_cast<const char*>(data), count * sizeof(T));
        }

        template<typename T>
        void set(const AVector<T>& data) {
            set(data.data(), data.size());
        }
    };

    class API_AUI_VIEWS IndexBuffer: public detail::VboImpl<gl::ResourceKind::INDEX_BUFFER> {
    private:
        size_t mIndicesCount;
    public:
        void bind();
        void set(const GLuint* indices, GLsizei length);

        void set(const AVector<GLuint>& indices) {
            set(indices.data(), static_cast<GLsizei>(indices.size() * sizeof(GLuint)));
        }

        void draw(GLenum primitiveType) {
            bind();
            glDrawElements(primitiveType, GLsizei(mIndicesCount), GL_UNSIGNED_INT, nullptr);
            glGetError();
        }

        [[nodiscard]]
        size_t count() const {
            return mIndicesCount;
        }
    };
}