#pragma once

#include <small_gl/detail/math.hpp>
#include <small_gl/enums.hpp>

namespace gl {
  // Declare swap-based move constr/operators for object T
  // and delete copy constr/operators, making T non-copyable
  #define gl_declare_noncopyable(T)\
    T(const T &) = delete;\
    T & operator= (const T &) = delete;\
    T(T &&o) noexcept { swap(o); }\
    inline T & operator= (T &&o) noexcept { swap(o); return *this; }

  // Non-templated OpenGL object wrappers
  class Buffer;
  class Fence;
  class Framebuffer;
  class Program;
  class Sampler;
  class Shader;
  class Vertexarray;
  class Window;
  class Query;

  // Templated OpenGL object wrappers
  template <typename T, 
            uint D,
            uint Components,
            TextureType Ty = TextureType::eBase>
  class Texture;

} // namespace gl