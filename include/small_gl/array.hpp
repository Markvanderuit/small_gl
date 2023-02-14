#pragma once

#include <small_gl/fwd.hpp>
#include <small_gl/detail/enum.hpp>
#include <small_gl/detail/handle.hpp>
#include <vector>

namespace gl {
  /**
   * Helper object to specify a vertex buffer for ArrayCreateInfo object.
   */
  struct VertexBufferCreateInfo {
    // Pointer to attached bufer object
    const Buffer *buffer;

    // Buffer binding point index
    uint index;

    // Offset to the first vertex element of the buffer, in bytes
    size_t offset = 0;

    // Distance between vertex elements in the buffer, in bytes
    size_t stride = sizeof(uint);

    // Rate at which vertex attributes advance in the case of instanced
    // rendering when reading from this buffer object
    size_t divisor = 0;
  };

  /**
   * Helper object to specify a vertex attribute for ArrayCreateInfo object.
   */
  struct VertexAttribCreateInfo {
    // Buffer/attrib binding point indices
    uint attrib_index, buffer_index;
    
    // Interpreted type of components in the vertex buffer binding
    VertexAttribType type = VertexAttribType::eFloat;

    // Nr. of components per element in the vertex buffer binding
    VertexAttribSize size = VertexAttribSize::e1;

    // Offset of the first vertex element to the start of the vertex
    // buffer binding this attribute fetches from, in bytes
    size_t offset = 0;
  };

  /**
   * Helper object to create Array object.
   */
  struct ArrayCreateInfo {
    // Vertex buffer binding info
    std::vector<VertexBufferCreateInfo> buffers;

    // Vertex attribute info
    std::vector<VertexAttribCreateInfo> attribs;

    // Optional elements buffer and internal type
    const Buffer  *elements      = nullptr;
    VertexElemType elements_type = VertexElemType::eUInt;
  };

  /**
   * Array object wrapping OpenGL vertex array object.
   */
  class Array : public detail::Handle<> {
    using Base = detail::Handle<>;

    bool           m_has_elements;
    VertexElemType m_elements_type;

  public:	
    /* constr/destr */
    
    Array() = default;
    Array(ArrayCreateInfo info);
    ~Array();

    /* getters/setters */

    inline bool has_elements() const  { return m_has_elements; }
    inline uint elements_type() const { return uint(m_elements_type); }

    /* state */

    void bind() const;
    void unbind() const;

    void attach_buffer(std::vector<VertexBufferCreateInfo> info);
    void attach_attrib(std::vector<VertexAttribCreateInfo> info);
    void attach_elements(const Buffer &buffer);
    void detach_elements();

    /* miscellaneous */
    
    inline void swap(Array &o) {
      gl_trace();
      using std::swap;
      Base::swap(o);
      swap(m_has_elements, o.m_has_elements);
    }

    inline bool operator==(const Array &o) const {
      return Base::operator==(o) && m_has_elements == o.m_has_elements;
    }

    gl_declare_noncopyable(Array)
  };
} // namespace gl