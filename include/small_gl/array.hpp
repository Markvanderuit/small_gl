#pragma once

#include <small_gl/detail/fwd.hpp>
#include <small_gl/detail/handle.hpp>
#include <vector>

namespace gl {
  /**
   * Helper object to specify a vertex buffer for ArrayCreateInfo
   */
  struct VertexBufferInfo {
    const Buffer *buffer;
    uint binding;
    uint offset = 0;
    uint stride = 4;
  };

  /**
   * Helper object to specify a vertex attribute for ArrayCreateInfo
   */
  struct VertexAttributeInfo {
    uint attribute_binding;
    uint buffer_binding;
    
    VertexFormatType format_type = VertexFormatType::eFloat;
    VertexFormatSize format_size = VertexFormatSize::e1;

    uint relative_offset = 0;
    bool normalize_data = false;
  };

  /**
   * Helper object to create array object.
   */
  struct ArrayCreateInfo {
    std::vector<VertexBufferInfo> buffers;
    std::vector<VertexAttributeInfo> attributes;
    const Buffer *elements = nullptr;
  };

  /**
   * Array object wrapping OpenGL vertex array object.
   */
  struct Array : public detail::Handle<> {
    /* constr/destr */
    
    Array() = default;
    Array(ArrayCreateInfo info);
    ~Array();

    /* getters/setters */

    inline bool has_elements() const { return _has_elements; }

    /* state */

    void bind() const;
    void unbind() const;

  private:
    using Base = detail::Handle<>;

    bool _has_elements;

  public:
    inline void swap(Array &o) {
      using std::swap;
      Base::swap(o);
      swap(_has_elements, o._has_elements);
    }

    inline bool operator==(const Array &o) const {
      return Base::operator==(o) && _has_elements == o._has_elements;
    }

    gl_declare_noncopyable(Array)
  };
} // namespace gl