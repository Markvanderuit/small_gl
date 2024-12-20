#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/utility.hpp>
#include <algorithm>

namespace gl {
  namespace detail {
    void attach_buffer(GLuint object, VertexBufferInfo info) {
      gl_trace_full();
      glVertexArrayVertexBuffer(object, info.index, info.buffer->object(), info.offset, info.stride);
      glVertexArrayBindingDivisor(object, info.index, info.divisor);
    }

    void attach_attrib(GLuint object, VertexAttribInfo info) {
      gl_trace_full();
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

  Array::Array(ArrayInfo info)
  : Base(true), 
    m_has_elements(info.elements) {
    gl_trace_full();

    glCreateVertexArrays(1, &m_object);
    
    // Bind vertex buffer objects and vertex attributes
    attach_buffer(info.buffers);
    attach_attrib(info.attribs);

    // Bind elements buffer, if provided
    if (m_has_elements) {
      m_elements_type = info.elements_type;
      attach_elements(*(info.elements));
    }
  }

  Array::~Array() {
    gl_trace_full();
    guard(m_is_init);
    glDeleteVertexArrays(1, &m_object);
  }

  void Array::bind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(m_object);
  }

  void Array::unbind() const {
    gl_trace_full();
    debug::check_expr(m_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(0);
  }
  
  void Array::attach_buffer(std::vector<VertexBufferInfo> info) {
    std::ranges::for_each(info, [&](auto &_info) { detail::attach_buffer(m_object, _info); });
  }

  void Array::attach_attrib(std::vector<VertexAttribInfo> info) {
    std::ranges::for_each(info, [&](auto &_info) { detail::attach_attrib(m_object, _info); });
  }

  void Array::attach_elements(const Buffer &elements) {
    m_has_elements = true;
    glVertexArrayElementBuffer(m_object, elements.object());
  }

  void Array::detach_elements() {
    m_has_elements = false;
    glVertexArrayElementBuffer(m_object, 0);
  }
} // namespace gl