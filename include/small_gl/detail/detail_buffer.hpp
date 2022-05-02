#pragma once

#include <glad/glad.h>

namespace gl::detail {
  GLint get_buffer_param_iv(GLuint object, GLenum name) {
    GLint value;
    glGetNamedBufferParameteriv(object, name, &value);
    return value;
  }
}