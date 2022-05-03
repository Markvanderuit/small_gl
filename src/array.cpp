#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/exception.hpp>

namespace gl {
  Array::Array(ArrayCreateInfo info)
  : Base(true), 
    _has_elements(info.elements) {
    expr_check(info.buffers.size() > 0, "no vertex buffer info was provided");
    expr_check(info.attributes.size() > 0, "no vertex attribute info was provided");

    glCreateVertexArrays(1, &_object);

    // Bind vertex buffer objects to vao
    for (const auto &info : info.buffers) {
      glVertexArrayVertexBuffer(_object,
                                info.binding,
                                info.buffer->object(),
                                info.offset, 
                                info.stride);
    }

    // Bind element buffer object to vao, if exists
    if (_has_elements) {
      glVertexArrayElementBuffer(_object, info.elements->object());
    }

    // Set vertex attrib formats and their bindings
    for (const auto &info : info.attributes) {
      glEnableVertexArrayAttrib(_object, info.attribute_binding);
      glVertexArrayAttribFormat(_object, 
                                info.attribute_binding,
                                (uint) info.format_size,
                                (uint) info.format_type,
                                info.normalize_data,
                                info.relative_offset);
      glVertexArrayAttribBinding(_object, info.attribute_binding, info.buffer_binding);
    }

    gl_check();
  }


  Array::~Array() {
    guard(_is_init);
    glDeleteVertexArrays(1, &_object);
  }

  void Array::bind() const {
    expr_check(_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(_object);
  }

  void Array::unbind() const {
    expr_check(_is_init, "attempt to use an uninitialized object");
    glBindVertexArray(0);
  }
} // namespace gl