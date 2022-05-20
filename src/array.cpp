#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/utility.hpp>
#include <algorithm>

namespace gl {
  namespace detail {
    void attach_buffer(GLuint object, VertexBufferCreateInfo info) {
      glVertexArrayVertexBuffer(object, info.index, info.buffer->object(), info.offset, info.stride);
    }

    void attach_attrib(GLuint object, VertexAttribCreateInfo info) {
      switch (info.type) {
        case VertexAttribType::eInt:
        case VertexAttribType::eUInt:
        case VertexAttribType::eShort:
        case VertexAttribType::eUShort:
        case VertexAttribType::eUByte:
        case VertexAttribType::eByte:
          glVertexArrayAttribIFormat(object, info.attrib_index, 
            (uint) info.size, (uint) info.type, info.offset);
          break;
        case VertexAttribType::eHalf:
        case VertexAttribType::eFloat:
          glVertexArrayAttribFormat(object, info.attrib_index,
            (uint) info.size, (uint) info.type, GL_FALSE, info.offset);
          break;
        case VertexAttribType::eDouble:
          glVertexArrayAttribLFormat(object, info.attrib_index,
            (uint) info.size, (uint) info.type, info.offset);
          break;
      }
      glVertexArrayAttribBinding(object, info.attrib_index, info.buffer_index);
      glEnableVertexArrayAttrib(object, info.attrib_index);
    }
  } // namespace detail

  Array::Array(ArrayCreateInfo info)
  : Base(true), 
    m_has_elements(info.elements) {
    debug::check_expr(info.buffers.size() > 0, "no vertex buffer info was provided");
    debug::check_expr(info.attribs.size() > 0, "no vertex attribute info was provided");

    glCreateVertexArrays(1, &m_object);

    // Bind vertex buffer objects and vertex attributes
    std::ranges::for_each(info.buffers, [&](auto &info) { detail::attach_buffer(m_object, info); });
    std::ranges::for_each(info.attribs, [&](auto &info) { detail::attach_attrib(m_object, info); });

    // Bind elements buffer, if provided
    if (m_has_elements) {
      glVertexArrayElementBuffer(m_object, info.elements->object());
    }
  }

  Array::~Array() {
    guard(m_is_init);
    glDeleteVertexArrays(1, &m_object);
  }

  void Array::bind() const {
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(m_object);
  }

  void Array::unbind() const {
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(0);
  }
} // namespace gl